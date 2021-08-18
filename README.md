<h1 align="center">Stamd</h1>

<p align="center">A static markdown page generator written in C</p>

## About

Motivated by [stagit](https://git.codemadness.org/stagit/), I've decided to take on this challenge
primarily because document parsing always fascinated me as well as to have a
complete control over to looks of articles the program produces.

### [See it in action](https://dimitrijedobrota.com/articles)

### [See more information about development](https://dimitrijedobrota.com/articles/Stamd.html)

## Built With

- C

## Future Updates

- [X] Support the basic subset of markdown
- [X] Be able to make a index for articles
- [ ] Support Tables
- [ ] Tag system

## Usage

The source code can be obtained using with:

    $ git clone git://git.dimitrijedobrota.com/stamd.git

After that we need to use `make` to install it:

```
$ cd ./stamd
$ make
$ sudo make install
```

This will install `stamd` and `stamd-index` to be used anywhere in the system.<br>
If we run the commands now we will get a hint about their usage:

```
$ stamd
  stamd [-o outdir] articledir
  
$ stamd-index
  stamd-index articledir articledir ...
```

`stamd` requires that the article directory is provided. It looks for
`article.md` and `config.txt` inside the article directory in order to create
the output HTML file.

Providing output directory is optional with `-o outdir`
and it instructs `stamd` to place the HTML file in specified directory. If the
directory is not provided output will be placed in article directory.

***

`stamd-index` takes in arbitrary number of article directories and writes, to
the standard output, the list of all the articles provided to be used as index
HTML file.

***

You will also receive `create_articles.sh` script which I use to bulk create all of the articles and sort them by date.

If you want to use it for yourself, you will need to edit `blogdir` and `sitedir` to match your setup, and after that you can simple run:

    $ ./create_articles.sh

***

Config file, as of now accepts Title, Date and Language option. Every option should start in a new line and general format is:

        Option: value
        
I am looking for a way to get rid of the config file in the future, so that all
of the necessary data can be extracted from the markdown file itself
        


## Author

**Dimitrije Dobrota**

- [Profile](https://github.com/DimitrijeDobrota)
- [Email](mailto:mail@dimitrijedobrota.com")
- [Website](https://dimitrijedobrota.com)
- [Blog](https://dimitrijedobrota.com/articles)

## 🤝 Support

Contributions, issues, and feature requests are welcome!

Give a ⭐️ if you like this project!

