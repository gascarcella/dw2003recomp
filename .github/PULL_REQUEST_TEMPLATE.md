<!-- Thanks for contributing! Please read CONTRIBUTING.md first. Keep the pull request to one topic. -->

## What changed

<!-- What this does and why. Link the issue it fixes: "Fixes #123". -->

## Area

- [ ] Decompilation (matching C, names, types, config)
- [ ] PC port (`port/`)
- [ ] Launcher (`launcher/`)
- [ ] Tests or tools
- [ ] Documentation

## How it was tested

<!-- Commands you ran and their result. Fork pull requests get CI without the disc, so the byte-identical build and
the reference tests have to be run by you. -->

- [ ] `scripts/build.sh --check` passes (every game file byte-identical)
- [ ] `scripts/test.sh` passes
- [ ] `tools/venv/bin/python tools/port_inventory.py probe` passes
- [ ] Port/launcher changes: built and run (say how)

## Checklist

- [ ] No game data: no disc image, extracted files, `asm/`, assets, BIOS or SDK files
- [ ] Anything borrowed from another project is recorded in `docs/THIRD_PARTY.md`
- [ ] New names follow the conventions (`docs/MATCHING.md`) and are in `config/*symbol*.txt`
- [ ] Docs updated if behaviour or a decision changed
- [ ] AI-generated parts, if any, are said here and I have checked them
