```md
# Crypto_Cyphers

Open-source cryptography project with:
- a **C shared library** (algorithms + helper utilities) — *Apache-2.0*
- a **CLI program** (`main.c`) to run ciphers and utilities — *MIT*
- a **MATH/** folder with personal notes/PDFs explaining the math first, then mapping to C algorithms

> Educational focus: start from the **mathematical approach**, then implement clean, testable **C** code.

---

## Status

- ✅ Repo structure in place
- 🛠️ Implementing core primitives and classic ciphers
- 📚 Notes in `MATH/` are being written from scratch (my own perspective)

---

## Repository layout

```

D:.
├── .vscode/                  # editor settings (optional)
├── bin/                      # top-level build output (optional)
├── CLI/                      # CLI program (MIT)
│   ├── src/
│   │   ├── bin/              # CLI executable output (e.g., cypher.exe)
│   │   ├── main.c            # entry point for the CLI 'cypher' tool 
│   │   └──Makefile
│   ├── run.bat
│   └── LICENSE               # MIT
├── lib/                      # C library (Apache-2.0)
│   ├── src/
│   │   ├── cypher.c
│   │   ├── cypher.h
│   │   └──Makefile
│   ├── LICENSE               # Apache-2.0
│   └── NOTICE
├── LICENSES/                 # canonical copies of licenses
│   ├── Apache-2.0.txt
│   └── MIT.txt
├── MATH/                     # rewritten explanations/notes (PDFs)
├── LICENSE                   # pointer explaining per-folder licenses
├── install.bat               # (optional) convenience installers
├── uninstall.bat             # (optional)
└── README.md

````

---

## Building

### Windows (MSYS2 MinGW-w64)

1) Install **MSYS2** → https://www.msys2.org  
2) Open *MSYS2 MinGW x64* shell and install toolchain:
```bash
pacman -Syu --noconfirm
pacman -S --needed --noconfirm mingw-w64-x86_64-toolchain make
````

3. Build the **library**:

```bash
cd /d/CYPHER/lib
make              # produces .dll / .a depending on the Makefile
```

4. Build the **CLI**:

```bash
cd /d/CYPHER/CLI
make              # produces .\src\bin\cypher.exe
```

5. (Optional) Ensure the runtime can find GCC DLLs:

* Add `C:\msys64\mingw64\bin` to your **User PATH** or run the CLI from the *MinGW x64* shell.

> If your Makefiles place outputs elsewhere, adjust paths accordingly.

### Linux (GCC/Make)

```bash
sudo apt-get install build-essential make
cd lib && make
cd ../CLI && make
```

---

## Usage

```powershell
# Windows PowerShell (after building)
.\CLI\src\bin\cypher.exe --help
# examples will be added as algorithms land:
# .\CLI\src\bin\cypher.exe caesar --key 3 --in msg.txt --out enc.txt
```

Library (example):

```c
#include "cypher.h"

int main(void) {
    // TODO: example once API is finalized
    return 0;
}
```

---

## Math notes (`MATH/`)

* These PDFs are **original explanations** written by me (not copies).
* They follow the flow from *Cryptography and Network Security: Principles and Practice* (Stallings), then translate the math into C algorithms.
* **No verbatim text or figures** from the book are included; citations/attribution are provided below.

---

## License

This repository uses **per-folder licensing**:

* **Library (`/lib`)** — **Apache-2.0**. See [`lib/LICENSE`](./lib/LICENSE) and [`lib/NOTICE`](./lib/NOTICE).
* **CLI (`/CLI`)** — **MIT**. See [`CLI/LICENCE`](./CLI/LICENCE) *(or `LICENSE` depending on your filename)*.

Canonical texts are available in [`/LICENSES`](./LICENSES).
The top-level [`LICENSE`](./LICENSE) explains the layout.

Per-file SPDX headers:

```c
/* lib/… (Apache-2.0) */
 /* SPDX-License-Identifier: Apache-2.0 */

/* CLI/… (MIT) */
 /* SPDX-License-Identifier: MIT */
```

---

## Attribution

This project implements algorithms described in:

* **William Stallings**, *Cryptography and Network Security: Principles and Practice*.
  Used as a conceptual reference only. All code and notes are my own work.

If you add other references or third-party code, include their licenses in `LICENSES/` or `THIRD_PARTY_NOTICES/`.

---

## Security & Disclaimer

* For education and research. **Not** audited for production or safety-critical use.
* If you find a vulnerability, please open a private advisory or email: **[security@example.com](mailto:security@example.com)** (replace with your contact).

---

## Roadmap (draft)

* [ ] Number theory helpers (gcd, mod inverse, powmod, CRT)
* [ ] Classical ciphers (Caesar, Vigenère, affine, Hill)
* [ ] Modern primitives (PRNG seeding, padding, PKCS#7)
* [ ] Hashes (SHA-256) and MAC (HMAC)
* [ ] Block cipher (AES-128/192/256) + modes (CBC/CTR/GCM\*)
* [ ] CLI UX (subcommands, files/STDIN, hex/base64 I/O)
* [ ] Tests & vectors (NIST where applicable)

\* GCM involves authentication; correctness and side-channel safety matter.

---

## Contributing

* Open an issue for discussion before large changes.
* Follow SPDX headers and match the correct folder license.
* Keep algorithms well-commented and accompanied by test vectors.

---

```

### Notes for you
- In your screenshot, the CLI file is named `LICENCE` (British spelling). That’s fine—just make sure the README links to the exact filename or rename it to `LICENSE` for consistency.
- If you want, paste me your preferred author name/email and I’ll add them in the README and `lib/NOTICE`.
```
