# Log
The completed steps of the project, described in chronological order.

<hr/>
<br/>

### Reading GP3 files
To read a GP3 file the program opens a file stream, and sequentially reads the data according to the file format. The data is stored in an instance of the class "GPFile".\
The GuitarPro 3 file format doesn't have any official documentation, but it has been reverse engineered for use in open source projects such as TuxGuitar. One such project, [PyGuitarPro](https://github.com/ergm569/PyGuitarPro), also provides some [documentation](https://pyguitarpro.readthedocs.io) for the file format.\
The documentation was not perfect, so I had to look at the source code a couple of times to make sure I got it right. But overall, it felt easier than just having to port the source code. Especially since those projects have more complex data models, made to work with multiple different versions or formats, which is out of the scope of this project.