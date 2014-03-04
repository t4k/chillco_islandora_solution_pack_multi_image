# Chillco Islandora Multi Image Solution Pack

## Introduction

A test implementation of containers holding images which consists of the following datastreams:
MODS (on the multi container object ONLY)
OBJ (mimetype image/tiff)
IMAGE (mimetype image/jpg,image/png,image/gif)
LARGE (mimetype image/jpg,image/png,image/gif)
MEDIUM (mimetype image/jpg,image/png,image/gif)
TN (mimetype image/jpg,image/png,image/gif)

It is based off the idea of the book solution pack in that the container holds the MODS record and some information about the first sequenced image. The other images are a part of the container that are related together via RELS-ext. You can use plupload to attach multiple images to an image container quickly.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Paged Content](https://github.com/islandora/islandora_paged_content)
* [Tuque](https://github.com/islandora/tuque)
* [Chillco Islandora Large Image Solution Pack](https://github.com/chillco/chillco_islandora_solution_pack_large_image)
* [Plupload](https://drupal.org/project/plupload)

## Installation

For the module, install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

## Configuration

Currently no configuration.

## Troubleshooting

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## FAQ

Why not use repurpose the compound solution pack?

We were initially thinking in this direction. What made us decide against it is that numerous changes would need to be made to the UI to make it work more like the book solution pack (batch add new images, manage set of images etc). The batch functionality in particular is what made more sense (instead of being able to add images ad-hoc as images cannot belong to more than one container). This may change down the line but for now, this was the approach.

It was also a fun exercise to learn and see how relations get made between objects :)

## Maintainers

[Ashok Modi](https://github.com/btmash)
