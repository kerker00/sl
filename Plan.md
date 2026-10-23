# Plan: sl — the ICE fleet edition

Status: draft, decisions taken (see section 9) · Branch: `plan/ice-fleet` · 2026-10-23

## 1. Goal

`sl` should be able to run **every ICE series** — and, after that, as many
other locomotives and trains as possible — across the terminal, while keeping
D51, C51, the little logo train and the TGV. We do not rewrite the project, but
we reshape it so that adding the next train is a matter of drawing ASCII art and
writing a few lines of data — not copying another 50-line `add_XXX()` function.

Guiding principles:

- **Keep the joke intact.** The binary stays `sl`, because the whole point is
  punishing a mistyped `ls`. Typing `sl` with no options still shows the D51 and
  cannot be interrupted. All existing flags keep working.
- **Stay small.** C, ncurses, a single binary, no runtime dependencies. Builds
  with a plain `make` on Linux and macOS.
- **Data over code.** A train is a description (cars, frames, colours, rules),
  not a function.
- **Testable.** Rendering is decoupled from curses so frames can be compared
  against golden files in CI.
- **Ready for the next life.** Adding a train, a car type or a feature touches
  one place, not every train.

## 2. Where we are today

State of `main` at v7.00 (2026-05-19): `sl.c` (~650 lines), `sl.h` (~240 lines
of ASCII art as `#define`s), `CHANGELOG.md`, CI that builds Linux (amd64) and
macOS (arm64) binaries and creates a GitHub release for `v*` tags.

What works:

- D51, C51, logo train (`-l`), TGV (`-G`), ICE 1 (`-i`) with power cars on both
  ends, n middle cars and a dining car when `n >= 8`, ICE 3 Redesign (`-3`) with
  n middle cars.
- `getopt_long` options, `-F` fly, `-L` land, `-a` accident, `-n` cars (1–100),
  `-r` continuous, `-b` backwards (all trains), `q` to quit (forward runs only).
- Already fixed in v7.00: `-b` dispatches all trains, smoke buffer bounded,
  `SIGSTOP` call removed, help text corrected.

Problems found while reading the code:

| # | Issue | Where |
| --- | ----- | ----- |
| 1 | One `add_XXX()` per train with copy-pasted logic for fly/land/accident/cars; every new train repeats it (ICE3RD just did) | `sl.c` |
| 2 | Features only exist for some trains: `-L` not ICE/ICE3RD/TGV, `-a` not ICE/ICE3RD, `-n` ignored by TGV | `add_*` |
| 3 | Train selection is an `if/else` chain duplicated in the forward and backward loop | `main` |
| 4 | `until` only knows ICE 1 and ICE3RD lengths; other trains use the ICE 1 length | `main` |
| 5 | `NUMBER` is mutated inside the draw functions as a default | `add_sl`, `add_D51`, `add_C51` |
| 6 | `q` quits forward runs, which breaks the "cannot escape" joke of plain `sl`; in `-b -r` there is no way out at all | `main` |
| 7 | Once 999 smoke puffs exist, no new smoke appears (long `-r` runs) — needs a ring buffer | `add_smoke` |
| 8 | ICE/ICE3RD set `bkgd()` to white for the whole screen; colour setup duplicated; no fallback without colours | `main` |
| 9 | Speed (`usleep`) hard-coded per train in two places | `main` |
| 10 | `sl.h` header still says 5.02; man page (`sl.1`, `sl.1.ja`) lacks the new options | `sl.h`, `sl.1` |
| 11 | Stray file `Untitled-1.txt`; CI without `-Wextra -Werror` | repo root, `.github` |
| 12 | ICE 1 art comes from ascii.co.uk — origin and licence of the art should be clarified | `README.md` |

## 3. Target architecture

```text
src/
  main.c          options, main loop, timing, input
  options.c/.h    CLI parsing (new + legacy flags)
  train.h         data model (struct train_def, struct car_def, ...)
  engine.c/.h     composes a consist and computes positions per frame
  render.h        canvas interface: put_str(y, x, str, colour), clear, flush
  render_curses.c ncurses backend (default)
  render_buffer.c in-memory backend for tests
  effects.c       smoke, accident people
  trains/
    registry.c    list of all trains, names and aliases
    d51.c c51.c logo.c tgv.c
    ice1.c ice2.c ice3.c ice4.c icet.c ...
art/              plain-text ASCII art, one file per car and frame
tests/            golden frames + small test runner
```

### 3.1 Data model (sketch)

```c
struct car_def {
    const char *name;          /* "powercar", "middle", "dining", "cab" */
    int height, width;
    int frames;                /* wheel animation frames */
    const char *const *art;    /* frames * height lines */
    const char *const *art_rev;/* optional: art for the opposite direction */
    int colour;                /* colour role, see 3.4 */
    int funnel_x;              /* -1 = no smoke */
    const struct man_pos *people; /* where -a puts people */
};

struct train_def {
    const char *id;            /* "ice4" */
    const char *aliases[4];    /* "412", "ice-4" */
    const char *title;         /* "ICE 4 (BR 412)" */
    int frame_divisor;         /* animation speed relative to x */
    int delay_us;              /* sleep per column */
    int default_cars, min_cars, max_cars;
    struct consist_rule rule;  /* how to build the consist from n */
};
```

The **consist rule** describes the order of cars, e.g. ICE 1:
`powercar · middle×n (dining at position 5 if n ≥ 8) · powercar(reversed)`;
ICE 2: `powercar · middle×n · cab car`; ICE 3/4: `end car · middle×n · end car`.
Optional: double traction (`--double`) couples two units.

### 3.2 One generic engine

A single `draw_train(def, x, opts)` replaces `add_D51/C51/TGV/ICE/sl`. It
handles for **every** train: number of cars, direction, fly, land, accident,
smoke. Issues 1–5 and 9 disappear with it.

### 3.3 Direction

Backwards (`-b`) works for every train. Each car may ship its own reversed art;
if it does not, it is mirrored automatically (`/`↔`\`, `(`↔`)`, `<`↔`>`, `[`↔`]`,
...) with protected labels like `DB` or `ICE` that keep their reading
direction. Hand-drawn reversed art always wins over mirroring.

### 3.4 Colours

Colour *roles* instead of fixed pairs: `body`, `stripe`, `window`, `wheel`,
`smoke`. Each train maps roles to colours (ICE: white body, traffic red stripe;
TGV: orange/grey). Without colour support, or with `NO_COLOR` set, everything
falls back to the default colours. No more global `bkgd()`.

### 3.5 Art as text files

The art lives in plain text files (`art/ice4/end.0.txt`) that contributors can
draw without escaping backslashes and quotes. A small `awk` step in the Makefile
generates `build/art.h` from them. No extra tools beyond POSIX.

### 3.6 Render backends

`render.h` exists so that frames can be tested without a terminal:

- `curses` – the real output.
- `buffer` – in-memory canvas for golden-frame tests, always built.

No further backends are planned.

## 4. CLI

New, unified:

```text
sl [options]
  -t, --train <name>   ice1, ice2, ice3, ice3rd, ice3m, ice3neo, ice4, icet, icetd, icel,
                       ice-v, d51 (default), c51, logo, tgv
      --list           list all trains with series number and year
  -n, --cars <n>       number of middle cars (clamped per train)
  -b, --backwards      left to right
  -F, --fly / -L, --land
  -a, --accident
  -r, --repeat         continuous; q quits (only way out)
      --double         double traction (where it exists)
      --random         random train
  -h, --help / -V, --version
```

Legacy flags stay as aliases: `-c` = `-t c51`, `-G` = `-t tgv`, `-i` = `-t ice1`,
`-l` = `-t logo`, `-3` = `-t ice3rd`, `--number` = `--cars`. Aliases also accept series numbers:
`sl -t 412`.

Input: without `-r`, `sl` ignores all input, including `q` (as upstream). With
`-r` the train never stops on its own, so `q` is the only way out there.

## 5. ICE fleet

To be verified and drawn one by one. Characteristic features are what the ASCII
art has to show so that people recognise the train.

| id | Series | In service | Consist | Recognisable by |
| --- | --- | --- | --- | --- |
| `ice-v` | BR 410.0 (InterCityExperimental) | 1985, prototype | 2 power cars + 3 test cars | Easter egg; record run 406.9 km/h |
| `ice1` | BR 401 | 1991 | 2 power cars + 12–14 cars, dining car | Raised dining car roof (done) |
| `ice2` | BR 402 | 1996 | 1 power car + 7 cars + cab car (808) | Asymmetric; often double traction |
| `icet` | BR 411 (7 cars) / 415 (5 cars) | 1999 | EMU, tilting | Lounge: view through the cab |
| `icetd` | BR 605 | 2001 | 4-car diesel EMU, tilting | Diesel, short |
| `ice3` | BR 403 | 2000 | 8-car EMU | Lounge, distributed traction |
| `ice3rd` | BR 403 Redesign | 2010s | 8-car EMU | Exists since v7.00 (`-3`); becomes a livery/variant of `ice3` |
| `ice3m` | BR 406 | 2000 | 8-car EMU, multi-system | Like 403, other markings |
| `ice-velaro` | BR 407 (Velaro D) | 2013 | 8-car EMU | Rounder Velaro nose |
| `ice4` | BR 412 | 2017 | 7, 12 or 13 cars | Flat, plain front, no lounge |
| `ice3neo` | BR 408 | 2022 | 8-car EMU | Velaro nose, new livery |
| `icel` | ICE L (Talgo, loco BR 105 / Vectron) | 2025, in regular service | Loco + low-floor Talgo cars + cab car | Low, short cars |
| `ice-s` | BR 410.1 | 1996, test train | Power car + test cars | Optional |

Liveries: `--livery classic` (ICE 1/2 era stripe) vs. `--livery current`
(traffic red). Optional, later.

### 5.1 Beyond the ICE

Once the ICE fleet is in, the goal is as many trains as possible. The data
model makes each one cheap. Candidates, in no particular order: BR 103 (TEE/IC
classic), BR 101, BR 218, BR 120, ET 403 ("Donald Duck"), BR 01 / BR 52 steam,
Rheingold, IC 2 double-deck (Twindexx / KISS), Railjet, Eurostar, Shinkansen
0/N700 (a nod to the Japanese origin of `sl`). Contributions welcome: one PR per
train, art plus data plus golden frames.

### 5.2 Art guidelines

 height ≤ 10 rows, uniform wheel frames per family, labels (`DB`,
`ICE`) at fixed positions so mirroring keeps them readable. All new art is
drawn by us; existing third-party art is either licensed properly or redrawn.

## 6. Phases

Each phase ends with a PR into `main` that builds and keeps the default
behaviour intact.

### Phase 0 – Housekeeping (small)

- Rename `master` to `main` (done 2026-10-23, CI trigger updated).
- Plain `sl` ignores input again; `q` only in `-r` mode (issue 6).
- Smoke as ring buffer (issue 7), remove `Untitled-1.txt`, update `sl.h` header
  and man pages (issue 10).
- CI: add `-Wextra -Werror`, `make install DESTDIR=` check.
- Makefile: `PREFIX`, `install`, `uninstall`, man page install.
- Release as v7.01 (tag only after explicit go).

### Phase 1 – Engine refactor with behaviour parity

- Introduce `render.h` with curses and buffer backends.
- Golden frames for D51, C51, logo, TGV, ICE 1, ICE3RD recorded *before* the refactor
  (fixed `COLS×LINES`, selected `x` positions).
- Move the six existing trains into the data model; the golden frames must not
  change (except for documented bug fixes).

### Phase 2 – Features for all trains

- `-b`, `-F`, `-L`, `-a`, `-n`, `-r` for every train, mirroring, colour roles,
  `NO_COLOR`, `--list`, `--train`, `--random`.
- Release as v8.00 (new CLI and engine).
- Update `sl.1`, README (English) and keep `sl.1.ja`/`README.ja.md` in sync for
  the original options.

### Phase 3 – The ICE fleet

- Art-as-text pipeline.
- One PR per series (or family): ICE 3 family first (403/406/407/408 share a
  lot), then ICE 4, ICE 2, ICE T/TD, ICE L, ICE V.
- Double traction for ICE 2/3/4.

### Phase 4 – More trains and extras

- Trains from section 5.1, one PR each.
- Fun: `--station` (train stops at a platform with a destination display,
  passengers board), coupling/splitting at a station, `--delay` easter egg.
- Packaging: Homebrew tap, Debian/Arch packages.

## 7. Testing

- `make test` builds a test binary that renders frames into the buffer backend
  and compares them with `tests/golden/*.txt`. No terminal needed, runs in CI.
- `make update-golden` regenerates the files deliberately.
- Each train gets at least: frame at the start, middle, end; backwards; fly.

## 8. Versioning and releases

- Versions follow the existing `vX.XX` scheme from `CHANGELOG.md`.
- Every PR adds a `CHANGELOG.md` entry; the release workflow takes the notes
  from there.
- Tags trigger a public GitHub release, so a tag is only pushed after an
  explicit yes for exactly that version.

## 9. Upstream and licence

- The original licence (Toyoda Masashi) stays; its notice must appear in all
  copies. New files get a short header that refers to it.
- Upstream `mtoyoda/sl` is quiet. Bug fixes from phase 0 could be offered there
  as small PRs; the ICE features stay in this fork.

## 10. Decisions

Taken on 2026-10-23:

1. **Name:** the binary and project stay `sl`. It is a joke on mistyping `ls`.
2. **Default train:** plain `sl` keeps the D51.
3. **Scope:** all ICE series including prototypes (ICE V, ICE S) and ICE L
   (already in regular service); after that, as many trains as possible (5.1).
4. **Input:** plain `sl` cannot be interrupted. `q` only works with `-r`.
5. **Displays:** no special display backends. The render seam is for tests only.
6. **Branch:** `master` is renamed to `main` (done on 2026-10-23).
