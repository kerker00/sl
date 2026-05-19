# Changelog

All notable changes to this project are documented here.
The version tag format is `vX.XX` (e.g. `v7.00`).

---

## [7.00] – 2026-05-19 – Markus Müller

- Add ICE 3 Redesign (ICE3RD) locomotive with full wagon support (`-3` / `--ICE3RD`)
- ICE3RD rear locomotive art (mirrored) for correct appearance in backward mode (`-b`)
- Code review & bug fixes:
  - Fix buffer overflow in `add_smoke()` (array `S[1000]` was unbounded)
  - Fix BACK-loop (`-b`) not dispatching D51 / C51 / TGV / sl trains
  - Add `-L` / `--land` to help text and `long_options[]`
  - Add `ICE3RDHEIGHT` constant (previously borrowed `ICEHEIGHT` from ICE 1)
- Help text: version number now shown in `sl -h` output
- Help text: corrected `-l` description; removed non-existent `-v` entry

## [6.01] – 2024-08-28 – Markus Müller

- ICE 1 can now drive forward and backwards (`-b` / `--backwards`)

## [6.00] – 2024-08-26 – Markus Müller

- Include many pull requests from GitHub
- Add ICE 1 locomotive with optional number of cars (`-i` / `--ICE`, `-n`)

## [5.03] – 2015-01-19 – Ryan Jacobs

- Fix some more compiler warnings

## [5.02] – 2014-06-03 – Jeff Schwab

- Fix compiler warnings

## [5.01] – 2014-01-03 – Chris Seymour

- Remove cursor; improve IO handling

## [5.00] – 2013-05-05 – Toyoda Masashi

- Add `-c` option (C51 locomotive)

## [4.00] – 2002-12-31 – Toyoda Masashi

- Add C51 locomotive; add `usleep(40000)`

## [3.03] – 1998-07-22 – Toyoda Masashi

- Add `usleep(20000)`

## [3.02] – 1993-01-19 – Toyoda Masashi

- D51 flies! Change options

## [3.01] – 1992-12-25 – Toyoda Masashi

- Wheel turns smoother

## [3.00] – 1992-12-24 – Toyoda Masashi

- Add D51 option

## [2.02] – 1992-12-17 – Toyoda Masashi

- Bug fixed (dust remains in screen)

## [2.01] – 1992-12-16 – Toyoda Masashi

- Smoke runs and disappears
- Change `-a` to accident option

## [2.00] – 1992-12-15 – Toyoda Masashi

- Add `-a` (all), `-l` (long), `-F` (Fly!) options

## [1.02] – 1992-12-14 – Toyoda Masashi

- Add turning wheel

## [1.01] – 1992-12-14 – Toyoda Masashi

- Add more complex smoke

## [1.00] – 1992-12-11 – Toyoda Masashi

- SL runs vomiting out smoke
