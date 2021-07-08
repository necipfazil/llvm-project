==================
Call Graph Section
==================

Introduction
============

With ``-fcall-graph-section``, the compiler will create a call graph section 
in the binary. It will include type identifiers for indirect calls and 
targets. This information can be used to map indirect calls to their receivers 
with matching types. A complete and high-precision call graph can be 
reconstructed by complementing this information with disassembly 
(see ``llvm-objdump --call-graph-info``).

Semantics
=========

A coarse-grained, type-agnostic call graph may allow indirect calls to target
any function in the program. This approach ensures completeness since no
indirect call edge is missed. However, it is generally poor in precision
due to having a much bigger than actual call graph with infeasible indirect
call edges.

A call graph section provides type identifiers for indirect calls and targets.
This information can be used to restrict the receivers of an indirect call to
indirect targets with matching type. Consequently, the precision for indirect
call edges are improved while maintaining the completeness.

The ``llvm-objdump`` utility provides a ``-call-graph-info`` option to extract
full call graph information by parsing the content of the call graph section
and disassembling the program for complementary information, e.g., direct
calls.

Section layout
==============

A call graph section consists of zero or more call graph entries.
Each entry has a list of indirect calls and targets with a type id.

An entry of a call graph section has the following layout in the binary:

.. csv-table:: Layout for a call graph section entry
  :header: Element, Content

  FormatVersionNumber, Format version number for the entry.
  TypeId, Type identifier for the list of indirect calls and targets that follow.
  CallSitePcCount, Number of indirect call site PCs that follow.
  CallSitePcs, List of indirect call site PCs.
  FuncEntryPcCount, Number of indirect target entry PCs that follow.
  FuncEntryPcs, List of indirect target entry PCs.

Each element in a call graph entry (including each element of the contained
lists) occupies 64-bit space.

The format version number is repeated per entry to support concatenation of
call graph sections with different format versions by the linker.

A type identifier may be repeated in different entries. The id value 0 is
reserved for unknown and used for indirect targets with unknown type.

As of now, the only supported format version is described above and has version
number 0.

Type identifiers
================

The type for an indirect call or target is the function signature.
The mapping from a type to an identifier is an ABI detail.
In the current experimental implementation, an identifier of type T is
computed as follows:

  -  Obtain the generalized mangled name for “typeinfo name for T”.
  -  Compute MD5 hash of the name as a string.
  -  Reinterpret the first 8 bytes of the hash as a little-endian 64-bit integer.

To avoid mismatched pointer types, generalizations are applied.
Pointers in return and argument types are treated as equivalent as long as the
qualifiers for the type they point to match.
For example, ``char*``, ``char**``, and ``int*`` are considered equivalent
types. However, ``char*`` and ``const char*`` are considered separate types.

Missing type identifiers
========================

If the compiler cannot deduce a type id for an indirect target, it will have a
special type id (0) assigned. These functions need to be taken as receiver to
any indirect call regardless of their type id. The use of listing such indirect
targets is to limit the receivers of indirect calls, which provides additional
precision by eliminating functions that cannot be target to indirect calls.

Similarly, call graph section does not guarantee a type id for all indirect
calls in the binary due to metadata loss. For completeness of the reconstructed
call graph, these calls must be taken to target any indirect target regardless
of their type id.

TODO: measure and report the ratio of missed type ids

Performance
===========

A call graph section does not affect the executable code and does not occupy
memory during process execution. Therefore, there is no performance overhead.

The scheme has not yet been optimized for binary size.

TODO: measure and report the increase in the binary size

Example
=======

For example, consider the following C code:

.. code-block:: c

    // Indirect target 1
    int foo(char a, float *b) {
      return 0;
    }

    // Indirect target 2
    int main() {
      char a;
      float b;

      // Indirect call site
      int (*fp_foo)(char, float*) = foo;
      fp_foo(a, &b);

      // Direct call site
      foo(a, &b);

      return 0;
    }

Following will compile it with a call graph section created in the binary:

.. code-block:: bash

  $ clang -fcall-graph-section example.c

During the construction of the call graph section, the type identifiers are 
computed as follows:

.. csv-table:: Function signature to numeric type id mapping
  :header: "Function name", "Generalized signature", "Mangled name (itanium ABI)", "Numeric type id (md5 hash)"

  "foo", "int (char, void*)", "_ZTSFicPvE.generalized", "e3804d2a7f2b03fe"
  "main", "int ()", "_ZTSFiE.generalized", "fa6809609a76afca"

Consequently, the call graph section will have the following content:

.. csv-table:: Call graph section content
  :header: Format version, Type id, Call site count, Call site list, Func entry count, Func entry list

  0, NumericTypeId(foo), 0, (empty), 1, FuncEntryPc(foo)
  0, NumericTypeId(foo), 1, CallSitePc(fp_foo()), 0, (empty)
  0, NumericTypeId(main), 0, (empty), 1, FuncEntry(main)

Notice that the current implementation may have seperate entries with the same
type id as above.

The ``llvm-objdump`` utility can parse the call graph section and disassemble
the program to provide complete call graph information. This includes any
additional call sites from the binary:

.. code-block:: bash

    $ llvm-objdump --call-graph-info a.out

    a.out:  file format elf64-x86-64

    INDIRECT TARGETS TYPES (TYPEID [FUNC_ADDR,])
    fa6809609a76afca 401130
    e3804d2a7f2b03fe 401110

    INDIRECT CALLS TYPES (TYPEID [CALL_SITE_ADDR,])
    e3804d2a7f2b03fe 40115b

    INDIRECT CALL SITES (CALLER_ADDR [CALL_SITE_ADDR,])
    401000 401012
    401020 40104a
    401130 40115b
    401170 4011b5

    DIRECT CALL SITES (CALLER_ADDR [(CALL_SITE_ADDR, TARGET_ADDR),])
    4010d0 4010e2 401060
    401130 401168 401110
    401170 40119d 401000

    FUNCTION SYMBOLS (FUNC_ENTRY_ADDR, SYM_NAME)
    401100 frame_dummy
    0 __libc_start_main@GLIBC_2.2.5
    401000 _init
    401090 register_tm_clones
    4010d0 __do_global_dtors_aux
    4011d0 __libc_csu_fini
    401110 foo
    401050 _dl_relocate_static_pie
    401060 deregister_tm_clones
    401020 _start
    4011d4 _fini
    401170 __libc_csu_init
    401130 main
