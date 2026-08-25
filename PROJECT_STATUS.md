# Project Status

**Repository packaging:** complete

The uploaded project archive was reorganized for version control without committing Vivado/Vitis generated cache and build directories.

### Included
- Vivado project (`.xpr`)
- Block design (`.bd`) and editable IP configuration (`.xci`)
- Exported hardware handoff (`.xsa`)
- Standalone ARM application source
- Application build metadata
- Documentation and verification notes
- Project-ready `.gitignore`

### Intentionally excluded
- Vivado cache/generated/run directories
- Vitis build output
- IDE workspace metadata
- tool logs and temporary files

These exclusions are version-control hygiene; they do not remove the editable hardware/software sources.
