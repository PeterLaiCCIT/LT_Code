
-----------------------------
LT Encoding library:
-----------------------------

This is a library that performs LT-style encoding and decoding of packets according to any distribution
(taken as input and read from a text file).

A few things to keep in mind:

- The number of input symbols, i.e., input packets to be encoded, is referred to as K. 

- The distribution is read from a text input file. 

- A number of example distributions, optimized for operation in Wireless Sensor Networks
can be found in the directory "distr". Each of these distributions is named as distr_xx.dd, 
where xx corresponds to the value of K, i.e., the number of input symbols this specific 
distribution is designed for.

- Distributions are saved (and used) according to the following format: all elements are 
integer numbers (16-bit) and their sum gives 65535. To obtain such distributions we started
from the wanted theoretical distributions with elements in [0,1] and converted them to their
quantized equivalent according to our 16-bit representation. In case you should need them, 
awk files to perform such conversions are in the directory "awk".

- The library performs encoding of symbols according to the given distribution, where the
encoding operation works through the bit-wise XOR of packets. Decoding is performed 
using Gaussian Elimination.

- Note that this is not just a simulator but a library that performs actual encoding and decoding.
In fact, it takes real packets as input, processes them and gives the full encoded packets as output
(and not just their encoding vectors). Same holds at the decoder, that performs the needed operations
on real data structures. Hence, this library may be used within any transmission system. 

- For the sake of illustration, I realized a sample main file, called "my-decoder.cpp" that uses the
library. This executable takes as input a number of parameters. See next for their explanation:

This is how you should call the executable file:
./my-decoder K bytes-per-packet distFile simID verbose #seeds

These are its input parameters:
- K: # of packets in input file
- SymbSize: size in bytes of each native packet
- distFile: encoding distribution, here you must write the complete path
- simID: simulation ID
- #seeds: number of random seeds to simulate

This example simply generates K packets of SymbSize bytes each, stores them into memory 
and fills them randomly. After that, these K packets are used to generate K+H encoded packets
(encoded using the selected input distribution). H says the number of  "redundant" encoded 
packets to generate (H is a general variable that is initialized at the beginning of the main, 
if you want you can modify the code to read it as an input arguments, as I did for K). These 
packets are obtained along with an encoding matrix G. When all of this is done, we simply 
call the decoding procedure. This procedure takes as input the encoded packets y and 
the encoding matrix G and returns the original native packets x = G^{-1} y. We measure 
the time taken by the entire procedure (encoding + decoding) for a number of random 
seeds (used to initialize the random number generator used to encode packets, i.e., pick 
packet degrees and symbols to combine). We repeat the encoding + decoding cycle for a 
number of random seeds (this number is given by the variable #seeds). At the end of it all, 
we save into the output file "complexity_GF2.txt" the average decoding time [usec].

For educational purposes:
This exmaple can be modified, for example increasing the number of redundant packets
(increase H), adding a packet error probability according to any error process, etc. 
So one might use this as a starting point for, e.g., a simulator of multicast 
transmission to a number of wireless users; transmit data adapting H "on the fly" 
(being a fountain code, the code is rateless so you can surely do this) and come
up with your protocol, etc. Also, you can implement a message passing decoder and
see how things work varying K (from small, i.e., less than 100 to large, i.e., 
thoiusands); measure the decoding complexities (in terms of number of XOR operations 
 and time taken to decode) of message passing and Gaussian Elimination and so forth..

To run the example simply:
- Type "make"
- Type "run_example" (is a shell script that uses "my-decoder"). Note "run_example" 
should be an executable file. If it is not, then simply type "chmod 777 run_example" 
to make it executable. 
- If you have "gnuplot" installed (you should, if you haven't please install it),
type "gnuplot example_graph.gnu" -> this script should automatically generate the 
performance graph "example_graph.eps" (in abscissa you have K and the y-axis shows 
the decoding time in usec).

The code and the example was explicitly written for a Linux/Unix OS environment. If you 
use Windows, I'm afraid you should port the code yourself. While in general I do not 
recommend using Windows for this type of things you are of course free to do it.

Have fun,

Michele Rossi, 31th of March 2010   //read by me
