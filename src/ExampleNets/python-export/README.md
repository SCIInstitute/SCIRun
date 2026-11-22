# Networks exported to Python

Sample output of **File > Export as Python Script...** (or `scirun_export_python(filename)`),
generated from the IBBM2015 tutorial networks of the same name in `../IBBM2015/`.

Each script rebuilds its network through the `scirun_*` Python API. Run one with
`scirun -s <script>.py`, or paste it into the SCIRun Python console.

**These files may be stale.** They were generated once and are not regenerated when the
exporter, the modules' state, or the source networks change. To see current output,
load the network and export it again.

They were exported headless, so they carry no module positions and their header omits
the source file name; an export from the GUI includes both.

Running them prints a short "Saved state not applied" list: the source networks carry a few
state keys from older SCIRun versions that today's modules no longer define.
