# segModule

**Modular Code Redesign with Testing Framework — Section: Image Segmentation**

This module is one section of a larger effort: redesigning the codebase around clean,
modular boundaries and standing up a real testing framework alongside it. Image
segmentation is the section currently being built out. The segmentation is the
*vehicle*; the deliverable is the **architecture** (factory pattern, config-driven
design via YAML, a generic feature model) and the **test harness** (GTest) that
proves the pieces hold together.

If you want the *why* behind the design — the layering, the bagLoader seam, what is
genuinely integrated vs. stubbed — read [`ARCHITECTURE.md`](ARCHITECTURE.md). This
README is the *how*: how to build it, run it, and test it day to day.

---

## What's here

Two executables come out of one build:

| Target | What it is | Inputs |
|---|---|---|
| `basicseg` | The real program. Loads a `.mi.gz` bag through the coworker's bagLoader, then segments it. | a YAML config + a bag file |
| `segmenter_test` | The GTest suite. Synthetic in-memory images — no bagLoader, no boost, no real data. | none (fixtures are built in code) |

The shape of the code, top to bottom:

```
ConfigLoader (YAML)  ─►  SegmenterFactoryBase ─► Threshold / Canny factory
                                  │
                                  ▼
                         SegmenterBase (abstract)
                           └─ CommonSegmenter
                                ├─ ThresholdSegmenter
                                └─ CannySegmenter
                                       │
                  SegImage ──► segment() ──► Objects (each carries named features)
```

`SegImage` is the image type segmentation runs on — a light `cv::Mat` wrapper that
stands in for the bagLoader's image class until that class is built out. The single
place the two worlds meet is `bag_adapter`. (Details in `ARCHITECTURE.md`.)

---

## Build

You need OpenCV, yaml-cpp, GTest, and boost (iostreams) — all already resolved by
`CMakeLists.txt` on this box.

First time / clean build:

```bash
cd /mnt/netapp/SECURITY/users/rnguyen/segModule/build
cmake ..
make
```

After that, you rarely need a full `make` — see the rebuild loop below.

---

## Running the tests (the part the group asks about)

The tests compile into **one standalone executable**: `build/segmenter_test`. The
mental model to share with everyone is:

> The test executable is just a file. You can run it from any directory — it only
> needs you to **rebuild** first so it reflects your latest changes.

One rule, and everything else follows.

### The one rule — rebuild, *then* run. Every time.

Editing `segmenter.cpp` or `segmenter_test.cpp` changes the *source*, not the
compiled binary. If you run the old executable without rebuilding, you are testing
old code — the single most common point of confusion.

`cmake --build` takes the build folder as an argument, so it works from **any**
directory — no `cd` first:

```bash
cmake --build /mnt/netapp/SECURITY/users/rnguyen/segModule/build --target segmenter_test
```

It recompiles only what changed, relinks the executable, and re-stages the test
config beside it. Then run the binary from wherever you are:

```bash
/mnt/netapp/SECURITY/users/rnguyen/segModule/build/segmenter_test
```

> **Why it works from anywhere:** the tests find their config fixture relative to the
> executable (not your current directory), and CMake copies `test_config.yaml` next
> to the binary on every build. So `build/`, the repo root, or `/tmp` all give the
> same result. The mechanics are in `ARCHITECTURE.md` if you're curious.

### The one-liner everyone should use

So nobody has to remember the path or the rule, drop this in your `~/.bashrc` — it
rebuilds then runs, in one command, from any directory:

```bash
alias segtest='cmake --build /mnt/netapp/SECURITY/users/rnguyen/segModule/build --target segmenter_test \
  && /mnt/netapp/SECURITY/users/rnguyen/segModule/build/segmenter_test'
```

Then, from literally any directory, after any change:

```bash
segtest
```

That single command *is* the workflow: edit code → `segtest` → read results.

### Handy GTest flags

These are baked into the executable — pass them straight through:

```bash
./build/segmenter_test --gtest_list_tests              # see every test
./build/segmenter_test --gtest_filter='ConfigLoader*'  # run a subset
./build/segmenter_test --gtest_filter='*Canny*'        # match by name
```

### What "passing" looks like right now

The current status is **11 passed / 6 failed** — the same from any directory — and
that is **expected**. The 6 failures are honest red — tests that feed deliberately
unwired/synthetic inputs into `segment()` (see the inline notes in
`tests/segmenter_test.cpp`). They are failures, not crashes, and they mark work that
is intentionally deferred at this concept stage. If you see a *different* count, or
crashes, that's worth a look — start by confirming you rebuilt before running.

---

## Running the program

`basicseg` takes a YAML config and a bag file:

```bash
cd /mnt/netapp/SECURITY/users/rnguyen/segModule
./build/basicseg tests/test_config.yaml \
    /mnt/netapp/SECURITY/users/ltang/public/TestGoogleTest/sample.mi.gz
```

The config picks the segmentation style (e.g. `Threshold` or `Canny`) and its
parameters; the factory builds the matching segmenter from it. Swap the style in the
YAML, no recompile needed — that's the config-driven design doing its job.

---

## Where to go next

- [`ARCHITECTURE.md`](ARCHITECTURE.md) — the design: layers, the bagLoader seam,
  what's integrated vs. stubbed, and the open decisions flagged for review.
- `tests/segmenter_test.cpp` — the test suite, with inline notes on the expected
  failures and what each fixture is exercising.
