# Contributing to CrewTrack

Thanks for your interest in making CrewTrack better! Here's how to contribute.

## Ways to Contribute

- **Report bugs** — Open an issue with steps to reproduce
- **Suggest features** — Open an issue describing what and why
- **Fix bugs** — Fork, fix, open a pull request
- **Add features** — Fork, implement, open a pull request
- **Improve docs** — Fix typos, add examples, translate
- **Test hardware** — Build it and report what works/doesn't

## Development Setup

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install ESP32 board support
3. Install libraries: `MFRC522`, `TFT_eSPI`, `ArduinoJson`
4. Configure `TFT_eSPI/User_Setup.h` as described in README
5. Open `CrewTrack.ino` and upload to ESP32

## Pull Request Process

1. Fork the repository
2. Create a branch: `git checkout -b feature/my-feature`
3. Make your changes
4. Test on real hardware if possible
5. Commit: `git commit -m "Add my feature"`
6. Push: `git push origin feature/my-feature`
7. Open a Pull Request with a clear description

## Code Style

- Use `camelCase` for variables and functions
- Use `UPPER_CASE` for `#define` constants
- Keep functions under 50 lines when possible
- Add comments for non-obvious logic
- No external libraries beyond what's already used

## Hardware Contributions

If you design a PCB, case, or accessory:
- Include KiCad/Fritzing source files
- Add photos of the assembled result
- Document BOM with supplier links

## Reporting Bugs

Include in your issue:
- ESP32 board model
- Arduino IDE version
- Library versions
- Wiring connections
- Serial monitor output
- What you expected vs what happened

## Feature Requests

Include:
- What problem it solves
- How you envision it working
- Whether you're willing to help implement it

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
