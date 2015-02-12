# Sauvegarde

Some experiments around backup and continuous data protection (CDP).

[![Build Status](https://travis-ci.org/dupgit/sauvegarde.png?branch=master)](https://travis-ci.org/dupgit/sauvegarde)
<a href="https://scan.coverity.com/projects/4150">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/4150/badge.svg"/>
</a>

## License

This is free software an real open source as GPLv3 is used for this
collection of programs. Feel free to contibute and/or give help to the
projects used here (see dependencies section).


## Dependencies

This project depends on :

* autotools     (2.59)
* glib and gio  (2.26)
* libmicrohttpd (0.9.3)
* libcurl       (7.40.0)
* sqlite        (3.6.20)
* jansson       (2.6)

doxygen (1.6.1) is used to generate code's documentation but is not
required to build the project. jansson's library version is quite recent
but it compiles nicely and depends on nothing (as far as I know).


## Roadmap

As I'm coding on spare hours I have no roadmap for now.


## Infrastucture

This is how I imagine the programs may interact themselves but it may
evolve in the future.

    hid   | -------------------------------------        -----------
    to    | | monitor + ciseaux + antememoire   |  <---> | serveur |
    user  | -------------------------------------        -----------
                                                               ^
    user                     ----------------                  |
    client                   | restauration | <----------------|
    (GUI ?)                  ----------------
    \                                          /          \          /
     ----- Client side (on a notebook) --------            - server -
                                                              side


* "monitor" may monitor a filesystem and send events to "ciseaux".
* "ciseaux" cuts files into pieces of 32768 bytes (by default) and transmits
  every pieces to "antememoire"
* "antememoire" stores everything in a local database before communicating
  with "serveur"'s main sauvegarde server.
* "serveur" is the main sauvegarde server. Each client communicates with it
  and it keeps every chunks of every files with their attributes.
* "restauration" is a tool that will provide the ability to restore some
  files or paths to some locations. It communicates directly with "serveur"
  main's sauvegarde server.


### Messages contents

Between threads messages are passed via pointers to C structures into
GAsyncQueue (glib). Binary data are not transformed and structures are
passed in place (no copy in memory).

Between programs messages are passed via a JSON structured message
(jansson). All binary data are transformed into some base64 encoded string.

msg_id field is used to identify message type. It is based on ENC_* macros
defined in packing.h. ENC_META_DATA indicates that the JSON string contains
a serveur_meta_data_t structure.


#### Directory

    G_FILE_TYPE_DIRECTORY
    user:group uid:gid
    access_time changed_time modified_time
    dir_mode
    dirname

As an example a JSON structure for a directory looks like :

    {
    "msg_id": 1, "hash_list": [], "filetype": 2, "group": "admin", "mode": 16877,
    "owner": "dup", "atime": 1401024480, "uid": 530, "ctime": 1401024479,
    "name": "/home/dup/Dossiers_Perso/projets/sauvegarde/monitor/.deps",
    "mtime": 1401024479, "gid": 530
    }


#### File

    G_FILE_TYPE_REGULAR
    user:group uid:gid
    access_time changed_time modified_time
    file_mode
    filename
        [
            -> number_of_chunk
            size
            checksum
        ]

As an example a JSON structure for a file looks like :

    {
    "hash_list": ["0CVUtILbsnYD3Royz7Yu8sOSxjl9f/b+b6wPa9DCRTY=", "AAXUaflNSSHEbZnjVAiY1b2Fz7YvQHwle/iZUHct09U=", "MunCygnx7AU/49Kdo+H3s72OludZ7KjzfA4k1ui2NAw="],
    "msg_id": 1, "filetype": 1, "group": "admin", "mode": 33188, "owner": "dup", "atime": 1401023677, "uid": 530, "ctime": 1401023685,
    "name": "/home/dup/Dossiers_Perso/projets/sauvegarde/config.log", "mtime": 1401023685, "gid": 530
    }


### Database (local cache)

For now Information is stored with the following scheme :

    --- files ----         -- buffers ---
    | file_id *  | <-1,n-> | file_id    |
    | type       |         | buf_order  |         --- data ----
    | file_user  |         | checksum   | <-n,1-> | checksum *|
    | file_group |         --------------         | size      |
    | uid        |                                | data      |
    | gid        |                                -------------
    | atime      |
    | ctime      |
    | mtime      |
    | mode       |
    | name       |
    --------------

Buffer order has to be kept. In the SQLITE cache we can keep it in an
explicit manner (by storing the order of the buffer of the checksum). In
the programs (when we pass things into memory with C structure or into
JSON formatted message) We keep buffer order in an implicit manner (by
storing the ordered list of checksums of a file). Fields marked with '*'
are primary keys.


## Coding into this project

### Comments

Sauvegarde uses Javadoc comment style to be parsed by doxygen. Please
continue to use this style into comments in the whole project as it will
be parsed by doxygen. doxygen has been configured to figure out which
parameters are not documented. Please have a look at doxygen output and
avoid any warning. If you want the documentation not to include the whole
path for files please have a look at sauvegarde.doxygen file and modify
STRIP_FROM_PATH variable with your path (where sauvegarde's sources are).


### Verifying complexity

I discovered 'lizard' a simple but excellent python program that checks the
complexity of a program (ok, how complex it look like because it does not
take into account preprocessor macros...). If you plan to hack into this
project please look at the result of this program (you may find it
following this link : [https://github.com/terryyin/lizard](https://github.com/terryyin/lizard))


## Learnt things from experiments

* GFileMonitor in Glib 2.26 does not warn when the file descriptors are
  exhausted. The default limit on my system is 1024 files monitored
* inotify interface does warn when one can not add a new watch. It says
  that the disk is full ! The limit on my system is 8192 watchs at a time.
* We should use another mechanism to monitor a whole filesystem as limits
  for GFileMonitor and inotify techniques are very low compared to the
  number of directories a filesystem might have (my /home have 37826
  directories !). We might want to look at redirfs
  (http://www.redirfs.org/tiki-index.php) or pluginfs
  (http://www.pluginfs.org/).
* redirfs, pluginfs and dazuko seems dead (on 02 march 2014).
* It seems (on my system) that the more the blocksize is small the more the
  intra-deduplication rate is high (from 2.56 % at 32768 bytes to 8% at 512
  bytes). But if the blocksize is smaller than cpu overhead is higher and
  the maximum filesystem size is smaller... For the default value we will
  have to find a value that will fit best intra-deduplication rate, cpu
  demand, maximum filesystem size and low memory consumption. For now the
  choosen value is 16384 bytes.
* It seems that it will be difficult to get things via 'simple' message
  passing between threads -> Use of GAsyncQueue that does all that is
  needed.
* I do not feel confortable with msgpack library. I miss documentation. I
  switched to jansson that is better documented and fills my needs.
* 0MQ is overkill for my needs. I switched to libcurl and libmicrohttpd.


## Usefull links

* Filesystem events notifycation :
 * redirfs        : [http://www.redirfs.org/tiki-index.php](http://www.redirfs.org/tiki-index.php)
 * pluginfs       : [http://www.pluginfs.org/](http://www.pluginfs.org/)
 * dazuko         : [http://dazuko.dnsalias.org/wiki/index.php/Main_Page](http://dazuko.dnsalias.org/wiki/index.php/Main_Page)
 * fanotify       : [http://www.xypron.de/projects/fanotify-manpages/man7/fanotify.7.html](http://www.xypron.de/projects/fanotify-manpages/man7/fanotify.7.html)
* sqlite          :
 * main site      :[http://www.sqlite.org/cintro.html](http://www.sqlite.org/cintro.html)
 * firefox add on : [https://addons.mozilla.org/fr/firefox/addon/sqlite-manager/](https://addons.mozilla.org/fr/firefox/addon/sqlite-manager/)
* zmq             : [http://czmq.zeromq.org/](http://czmq.zeromq.org/)
* data packing over the wire :
 * msgpack        : [http://msgpack.org/](http://msgpack.org/)
 * jansson        : [http://www.digip.org/jansson/](http://www.digip.org/jansson/). See also the RFC 7159: [http://tools.ietf.org/html/rfc7159.html](http://tools.ietf.org/html/rfc7159.html)
* doxygen         : [http://www.stack.nl/~dimitri/doxygen/index.html](http://www.stack.nl/~dimitri/doxygen/index.html)
* Software quality checkers :
 * lizard         : [https://github.com/terryyin/lizard](https://github.com/terryyin/lizard)
* Third party libraries :
* libmicrohttpd   : [https://www.gnu.org/software/libmicrohttpd/](https://www.gnu.org/software/libmicrohttpd/)
* libevent        : [http://libevent.org/](http://libevent.org/)

