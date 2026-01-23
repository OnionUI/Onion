# Shell Tests

Integration tests using Docker + BATS.

```bash
make test-shell        # Run all tests
make test-shell-debug  # Interactive debug
```

## Adding Tests

1. Create `<name>.bats` in appropriate folder (`package_manager/`, `packages/`, `theme_switcher/`)
2. Load helpers and use `setup()`:
   ```bash
   load '../../support/test_helper'
   load '../../support/mocks'
   
   setup() {
       cleanup_test_data
       mock_system_commands
   }
   ```
3. Run scripts with `run sh "$PROJECT/path/to/script.sh" args`

## Available Mocks

- `mock_system_commands` - No-ops `sync`, `sleep`; logs `reboot`, `poweroff`
- `mock_date [timestamp]` - Returns fixed timestamp
- `mock_md5sum [hash]` - Returns fixed hash
- `mock_binary <name>` - Creates logging stub
