# cryptodev-linux-comp (cryptodev-linux) with compression support

Build status: https://travis-ci.org/github/joanbm/cryptodev-linux

This repository contains a fork of [cryptodev-linux](http://cryptodev-linux.org/) with compression support, which allows accessing hardware compressors from userspace.

The main aim of this fork is allowing access to the 842 hardware compressor available on Intel POWER7+ machines. For more complete information, you can visit the [lib842 repository](https://github.com/joanbm/lib842).

The usual build instructions for building, installing and loading the cryptodev module apply to this fork, as no new dependencies have been introduced.
Additionally, 842 compression support is available on the mainline Linux kernel and will automatically use the hardware compressor on machines posessing it.

The original cryptodev README follows:

# cryptodev-linux

This is a `/dev/crypto` device driver, equivalent to those in OpenBSD or
FreeBSD. The main idea is to access existing ciphers in kernel space
from userspace, thus enabling the re-use of a hardware implementation of a
cipher.

For questions and suggestions, please use the homepage at https://github.com.
Cryptodev-linux is hosted at https://github.com/cryptodev-linux/cryptodev-linux.

Older releases are also available at http://cryptodev-linux.org.

## How to combine with cryptographic libraries

### GnuTLS

GnuTLS needs to be compiled with `--enable-cryptodev` in order to take
advantage of `/dev/crypto`. GnuTLS 3.0.14 or later is recommended.

### OpenSSL

OpenSSL needs `-DHAVE_CRYPTODEV` and `-DUSE_CRYPTODEV_DIGESTS` flags
during compilation. Note that the latter flag (digests) may induce
a performance penalty in some systems.

## Modifying and viewing verbosity at runtime

The verbosity of the driver often needs to be adjusted for debugging.
The `sysctl` tool can be used for that.

```
# sysctl ioctl.cryptodev_verbosity
ioctl.cryptodev_verbosity = 0

# sysctl ioctl.cryptodev_verbosity=3
ioctl.cryptodev_verbosity = 3
```
