"""Index static libraries with the toolchain's plain ranlib on RISC-V builds.

The platform sets RANLIB to riscv32-esp-elf-gcc-ranlib, a small wrapper that
loads GCC's LTO plugin. From 2026-10-26 Windows Smart App Control blocks that
one file on the development PC (unknown reputation) while the rest of the
toolchain runs. Nothing here is built with LTO, so the plain ranlib indexes
the archives identically.

This runs as a pre: script, before the platform's own build script, and sets
the whole ranlib command rather than RANLIB: the platform overwrites RANLIB
later, and the framework's library builders clone the environment while the
platform script runs, so a post: script would come too late for them. The
command names the tool by its base name, which the build finds on the
toolchain's PATH entry.
"""
Import("env")

mcu = str(env.BoardConfig().get("build.mcu", ""))
if mcu.startswith("esp32") and mcu not in ("esp32", "esp32s2", "esp32s3"):
    env.Replace(RANLIBCOM='riscv32-esp-elf-ranlib "$TARGET"')
