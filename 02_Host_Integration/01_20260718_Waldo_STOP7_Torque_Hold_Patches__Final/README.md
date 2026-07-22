# Waldo / PAROL6 API STOP7 compatibility patches

These patches adapt the host side to the Octopus Pro firmware's STOP7 behavior:
motion stops immediately, already-enabled drivers keep holding torque, and
release never causes automatic motion. Recovery is an explicit Reset followed
by a separate Resume/Enable action.

## Why both patches are required

Unpatched PAROL6 Python API v0.3.1 converts the physical E-stop input into
firmware `DISABLE` (command 102). On a gravity-loaded robot that releases driver
torque, which conflicts with this port's intentional torque-holding STOP7
contract. The Waldo patch adds the corresponding reset/verification UI flow and
prevents automatic resume when hardware telemetry first appears.

These patches are required for the documented STOP7 semantics. They do not turn
STOP7 into a certified safety system; keep a separate 24V power cutoff.

## Pinned upstream revisions

| Component | Revision |
|---|---|
| [PAROL6 Python API](https://github.com/PCrnjak/PAROL6-python-API) | v0.3.1 / `8f6b3d19ddd3fb854af9890d89c7c96ca093cb7c` |
| [Waldo Commander](https://github.com/Jepson2k/Waldo-Commander) | `72d7ced4da1a8b0ababd4ca9a39a0693523b098f` |

The same revisions are machine-readable in `upstream.lock`.

Patch digests are recorded in `SHA256SUMS.txt`. The Waldo patch includes the
deployed first-telemetry guard so an initial `io[4] == 0` sample is handled as a
startup safety state instead of being mistaken for a new button transition.

## Apply

Start from clean checkouts at the pinned commits.

```bash
git clone https://github.com/PCrnjak/PAROL6-python-API.git
cd PAROL6-python-API
git checkout 8f6b3d19ddd3fb854af9890d89c7c96ca093cb7c
git apply /absolute/path/to/patches/0001-parol6-api-v0.3.1-clear-error-103.patch
```

Then patch Waldo Commander:

```bash
git clone https://github.com/Jepson2k/Waldo-Commander.git
cd Waldo-Commander
git checkout 72d7ced4da1a8b0ababd4ca9a39a0693523b098f
git apply /absolute/path/to/patches/0002-waldo-commander-exp19-reset-handshake.patch
```

Install and run each project according to its upstream instructions. To prevent
Waldo from opening a camera, start it with `WALDO_DISABLE_CAMERA=1`.

## Recovery sequence

The host sends four ordered frames:

```text
IDLE (255) -> CLEAR_ERROR (103) -> IDLE (255) -> ENABLE (101)
```

The firmware accepts the reset edge only after STOP7 has physically returned to
the healthy closed state. Resume/Enable remains a separate deliberate action.

## Tests

Artifact-level tests in this directory do not need the upstream checkouts:

```bash
python3 -m unittest discover -s tests -p 'test_*.py'
```

For a full verification, also run each patched upstream project's own unit and
integration tests. `clear_error_contract.py` is a small review/test helper; it
is not required by the firmware at runtime.

## Known limits

- The web UI displays commanded output state; it is not electrical or pressure
  feedback from the pneumatic valve.
- USB disconnect and STOP7 do not provide a watchdog for FAN0/OUTPUT1, so the
  last output state may remain active.
- This release does not claim complete Better Command program-runner
  compatibility.
