<h1>"Compressed Sparse Row" Calculator</h1>
<p>
  This is a program written entirely in C that multiplies two matrices in the CSR-Format. CSR is beneficial in storing sparse matrices since it stores only non-zero elements thereby substantially reducing space for storing the matrices. <br>
  The compiled program offers switching between faster V0 and slower V1. Also, runtime can be measured optionally.
  <br><br>This was part of a group project in university.
</p>
<h2>What is CSR?</h2>
<p>Guide on the general format: https://pnxguide.medium.com/compressed-sparse-row-motivation-and-explanation-cd92c71b7cfa</p>
<h2>How do I use this calculator?</h2>
<p>
  Matrices are given to the calculator in .txt files. The format must exactly follow the specifications below. Numbers may be floats or integers. <br><br>
  Compilation: <br>
  Navigate to the "Code" directory. Make sure you have Make and a C compiler (e.g. gcc, clang) installed. Then simply run "make". This creates the executable calculator called "program". <br> <br>
  Usage: <br>
  -------------- <br>
  ./program [OPTIONS] <br>
  Options: <br>
  -V number -> Specify implementation version. Default is 0. <br>
  -B[number] -> Runtime measurement. Optionally, the number of repetitions can be specified. <br>
  -a filename -> Input file containing matrix A. <br>
  -b filename -> Input file containing matrix B. <br>
  -o filename -> Output file. <br>
  --help, -h   -> Display this help and exit. <br>
<br>
Examples: <br>
  ./program -a matrix_a.txt -b matrix_b.txt -o result.txt <br>
  ./program -V 1 -B5 -a matrix_a.txt -b matrix_b.txt -o result.txt <br>
  -------------- <br>
</p>
<h2>Text Input Format</h2>
<p>
  m,n <br>
  v1,v2,v3,...,vn <br>
  c1,c2,c3,...,cn <br>
  r1,r2,r3,...,rn <br>
</p>
