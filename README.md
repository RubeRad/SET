# SET

In the popular card game SET, there are 81 cards, each with a unique
combination of 3 possible values for 4 attributes (number: 1,2,3;
texture: solid, open, striped; color: red, green, purple, shape:
rectangle, oval, squiggle). A valid set is three cards with, for each
attribute, all 3 values the same, or all three different.

Simple combinatorics reveals that there are 1080 possible sets, and
C(81,3)=83500 total groups of 3 cards. Thus if n=3 cards are dealt out,

  1080  deals have 0 sets
  84240 deals have 1 set

Exact enumerations are known for a few small values of n, but not for
n=12, which the rules of SET specify as the starting deal. Randomized
simulations show that about 3.2% of all 12-card SET deals have no
sets. This project aims to figure out the exact number.

There are C(81,12)=70,724,320,184,700 (about 70 trillion) possible
12-card deals. In order to solve this problem within 24 hours will
require a pace of over 800 million deals/second. Hopefully we can get
within that order of magnitude using OpenCL to drive a GPU with a lot
of cores.

For more information and inspiration go read _The Joy of SET_ (Gordon&McMahon)!


To launch the jupyter notebook online, click here (may take a minute or two to
create the virtual environment):

[![Binder](https://mybinder.org/badge_logo.svg)](https://mybinder.org/v2/gh/RubeRad/SET/master)



