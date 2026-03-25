# cobol-llm
COBOL-LLM is a [GnuCOBOL](https://gnucobol.sourceforge.io/) library for accessing Large Language Model APIs,
including Anthropic Claude, Ollama (locally run open source models), and OpenAI-compatible endpoints.

## Motivation
Hundreds of billions of lines of COBOL continue to power critical systems in banking, insurance, and government.
This library brings modern LLM capabilities to that ecosystem — not as a replacement strategy,
but as a capability extension. COBOL systems can call LLMs directly, without middleware layers
written in other languages.

## Status
Early development. Not yet usable.

The project is currently in the design and initial implementation phase.
Watch or star the repo if you're interested in following progress.

## Architecture

- `lib/c/`: libcurl-based HTTP transport layer (C)
- `lib/copy/`: COBOL copybooks defining the public API data structures
- `lib/cob/`: COBOL source modules
- `lib/cob/providers/`: Provider-specific modules (Claude, Ollama, OpenAI-compatible)
- `tests/`: Unit and integration test suite
- `examples/`: Example programs

## Target Platform
GnuCOBOL 3.x on Linux. Other platforms may follow.

## Contributing
Too early for contributions, but feedback and ideas are welcome via Issues.

## License
GNU Lesser General Public License v2.1
