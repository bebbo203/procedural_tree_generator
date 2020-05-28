# MYOT
## Roberto Aureli 1757131

A space colonization based tool that generates tree of various types.
The project is heavely based on [Yocto](https://github.com/xelatihy/yocto-gl) and follows the concept on the [Modeling Trees with a Space Colonization Algorithm](http://algorithmicbotany.org/papers/colonization.egwnp2007.large.pdf) paper.

## Implementation

Following the Yocto philosophy, a tree is seen as a Shape.
Starting from a point given by the user, the branches grow to reach a  cloud of attractors (points) scattered in the space following a known rule.\
Contrarily to the paper, Voronoi's set are not used but, for semplicity (at cost of performances), only a distance function is taken from Yocto to map the set of active attractors with respect to a growing branch.

An attractor is *active* when it's sphere of influence contains a tree node that is growing.\
An attractor is *dead* (and then removed from the cloud) when it's under a certain distance from a node.

Starting from a generic tree node, the growth direction is decided by the average of the positions of active attractors in regard to that node. The length is a parameter.

Every node is considered in the growing loop untile no more attractor are alive or a certain number of branches is reached.

Initially a branch is a line that is then evolved to a mesh. The latter is a cylinder with an emisphere at the top used to fill the "holes" in the meshes that can be caused by the high angle of attacl of two adiacent branches.\
The width of a branch is given by the *Da Vinci's formula* that put in relations the n-th power of the width of a parent branch to the n-th power of the widths of the childs. \
To keep track of every width, a matrix is built after traversing all the tree.\
A further loop is finally executed along all the final branches to add leaves following a *Logaritmic Spiral Distribution*.

## Usage

There is a plenty of parameters to decide the shape of a generic tree so, for the command line interface, only a little part of them is editable by the user (while in the main function everything is editable and commented to fine tune the tree).

### Single model mode

    -s true -t < 1|2|3 > --s-textures <path> --s-textures-opacity <path> ...

This mode helps with keeping everything in a compact way. A single texture is mapped over the trunk and the leaves giving the possibility to use up to 3 differents textures for the latter.
Following a clockwise disposition, the leaves textures need to be displaced in the right order to be taken by the software. 
The first frame is for the trunk texture. 


### Multiple models mode

    -s false --m-trunk-texture <path> --m-leaves-textures <path> --m-leaves-textures-opacity <path> -t <N> 

With multiple models, the generator will return one object for the trunk and *N* different objects for the leaves. Every leaves object is mapped to one of the *N* textures.\
There are some rules to follow to build a tree with this modality regarding the naming of the leaves textures.\
Name the texture with a final number that starts from 0 and goes to *N*-1 but does not include it in the *--m-leaves-textures* and *--m-leaves-textures-opacity* paths.\
Example:

> File on the disk:
>>trunk.png\
>>leaf_0.png\
>>leaf_1.png\
>>leaf_opacity_0.png\
>>leaf_opacity_1.png\

> Parameters:
>>     -s false --m-trunk-texture "./trunk" --m-leaves-textures "./leaf_" --m-leaves-textures-opacity "./leaf_opacity_" -t 2 ...

Make sure to map exactly the index of the color texture to the one of the opacity texture.
