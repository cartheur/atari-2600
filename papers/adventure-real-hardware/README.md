# Camera-mediated Atari 2600 agent paper

This folder scopes a results-free arXiv paper draft for the repository's first
agent experiment: train in an emulator, then evaluate *Adventure* on an
original Atari 2600 from camera observations and external joystick contacts.

The draft is intentionally a study protocol plus paper skeleton.  Do not turn
the bracketed placeholders into claims until the corresponding runs, videos,
hardware configuration, and analysis notebooks exist.

## Paper claim to test

An Atari agent trained with camera/control-domain randomization can transfer to
an unmodified original Atari 2600 for *Adventure*, while remaining observable
and reproducible through synchronized camera/action logs.  This is not a claim
about general intelligence or human-level performance.

## Draft layout

- `main.tex` — arXiv-style manuscript skeleton.
- `references.bib` — foundational benchmark and reinforcement-learning
  references.

## Build

From this directory, run:

```sh
latexmk -pdf main.tex
```

If `latexmk` is unavailable, use `pdflatex main.tex`, `bibtex main`, then run
`pdflatex main.tex` twice.  Build output is intentionally ignored by the
repository's top-level workflow; `main.pdf` is retained as a review draft, not
as a submission artifact.

## Evidence required before submission

1. Exact ROM hash, game variation, console revision/region, display and camera
   configuration.
2. Controller bridge schematic, firmware revision, continuity checks and
   watchdog failure tests.
3. Emulator seeds, training budgets, model checkpoints and environment version.
4. Timestamped camera/action/relay logs and complete hardware-run videos.
5. Predeclared metrics, baseline results, confidence intervals, failures, and
   compute/energy accounting.
6. Rights review for ROMs, screenshots, gameplay footage, and any human data.

Repository context: [agent proposal](../../AGENT_PLAY_PROPOSAL.md) and
[hardware BoM](../../HARDWARE_BOM.md).
