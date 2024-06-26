               intel_dispatch_patch.zip
               ========================

By Agner Fog, Technical University of Denmark, 2019.

Intel's compilers are generating code that will run slower than necessary when
the code is executed on a CPU that is not produced by Intel. This has been
observed with Intel C, C++, and Fortran compilers.

The same happens when certain function libraries produced by Intel are used,
even if the code is compiled with another compiler, such as Microsoft, Gnu
or Clang compilers.

This problem is affecting several commonly used software programs such as 
Matlab, because they are using Intel software libraries.

The library code and the code generated by an Intel compiler may contain
multiple versions, each optimized for a particular instruction set extension.
A so-called CPU dispatcher is chosing the optimal version of the code at
runtime, based on which CPU it is running on.

CPU dispatchers can be fair or unfair. A fair CPU dispatcher is chosing the
optimal code based only on which instruction set extensions are supported
by the CPU. An unfair dispatcher first checks the CPU brand. If the brand
is not Intel, then the unfair dispatcher will chose the "generic" version 
of the code, i.e. the slowest version that is compatible with old CPUs 
without the relevant instruction set extensions.

The CPU dispatchers in many Intel function libraries have two versions, a 
fair and an unfair one. It is not clear when the fair dispatcher is used
and when the unfair dispatcher is used. My observations about fair and
unfair CPU dispatching are as follows:

* Code compiled with an Intel compiler will usually have unfair CPU dispatching.

* The SVML (Short Vector Math Library) and IPP (Intel Performance Primitives)
  function libraries from Intel are using the fair CPU dispatcher when used 
  with a non-Intel compiler.

* The MKL (Math Kernel Library) library contains both fair and unfair
  dispatchers. It is not clear which dispatcher is used on each function.

The code examples contained herein may be used for circumventing unfair CPU
dispatching in order to improve compatibility with non-Intel CPUs.

The following files are contained:

intel_cpu_feature_patch.c
-------------------------
This code makes sure the fair dispatcher is called instead of the unfair
one for code generated with an Intel compiler and for general Intel
function libraries.

intel_mkl_feature_patch.c
-------------------------
This does the same for the Intel MKL library.

intel_mkl_cpuid_patch.c
-----------------------
This code example is overriding CPU detection functions in Intel's MKL 
function library. The mkl_serv_intel_cpu() function in MKL is returning
1 when running on an Intel CPU and 0 when running on any other brand of
CPU. You may include this code to replace this function in MKL with a
function that returns 1 regardless of CPU brand.

It may be necessary to use both intel_mkl_feature_patch.c and 
intel_mkl_cpuid_patch.c when using the MKL library in software that
may run on any brand of CPU.

An alternative method is to set the environment variable
   MKL_DEBUG_CPU_TYPE=5
when running on an AMD processor. This may be useful when you do not have
access to the source code, for example when running Matlab software.

The patches provided here are based on undocumented features in Intel
function libraries. Use them at your own risk, and make sure to test your
code properly to make sure it works as intended.

The most reliable solution is, of course, to avoid Intel compilers and 
Intel function libraries in code that may run on other CPU brands such
as AMD and VIA. You may find other function libraries on the web, or 
you may make your own functions. My vector class library (VCL) is useful
for making mathematical functions that process multiple data in parallel,
using the vector processing features of modern CPUs.
