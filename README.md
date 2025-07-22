# HighFive: HighFive Headers for R

## About

This package provides [R](https://www.r-project.org) with access to
[HighFive](https://github.com/highfive-devs/highfive/) header files.  [HighFive](https://github.com/highfive-devs/highfive/)
provides an idiomatic Modern C++ interfacee to HDF5.

By providing the [HighFive](https://github.com/highfive-devs/highfive/) library in this package, we
offer a more efficient distribution system for [CRAN](https://cran.r-project.org)
as replication of this code in the sources of other packages is avoided.

To use it, simply add it to the `LinkingTo:` field in the `DESCRIPTION` field of your R
package---and the R package infrastructure tools will then know how to set
include flags correctly on all architectures supported by R.

NB: These header files require libhdf5 and a compiler implementing C++14 or greater,
which is, to my understanding, all compilers supported by modern R.

## See Also

The [issue tracker](https://github.com/theAeon/HighFive/issues)
can be used for bug reports or feature requests.

## Author

Andrew Robbins (with README and package structure shamelessly borrowed from Dirk Eddelbuettel)

### License

This package is provided under the same license as HighFive, the BSL-1.0
