# stamd

Static Markdown blogging platform that generates everything you need

## Description

With this project you can seamlessly turn a collection of .md files into a
static webpage, with auto generated indexes for categories, rss and atom feeds,
sitemap.xml and robots.txt.

It's configured to make pages for my [personal website](https://dimitrijedobrota.com/),
but as generation code is self documenting , it can be easily adopted to fit
your needs, all you have to do is change write_header and write_footer methods
as you see fit.

As of now most of the heavy lifting is done by md4c library, until I find time
to develop sophisticated Markdown to HTML converter.


## Dependencies

* CMake 3.14 or latter
* Compiler with C++20 support (tested: clang 16.0.5, gcc 13.2.0)
* [Poafloc latest](https://github.com/DimitrijeDobrota/poafloc)
* [Hemplate latest](https://github.com/DimitrijeDobrota/hemplate)
* [md4c](https://github.com/mity/md4c)


## Building and installing

See the [BUILDING](BUILDING.md) document.


## Usage

> Please reference [cabin-deploy.sh](https://github.com/DimitrijeDobrota/cabin) for relevant usage example


## Version History

- 0.3
    * Generate sitemap.xml and robots.txt
    * Generate rss.xml and atom.txt feeds
    * Configurable links
    * Category indexes
- 0.2
    * C++ rewrite
    * Improved stability
    * Improve readability

- 0.1
    * Quick and dirty proof of concept written in C


## Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.


# Licensing

This project is licensed under the MIT License
- see the [LICENSE](LICENSE.md) document for details
