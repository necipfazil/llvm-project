; Tests that we create function comdats properly and only for those with no
; comdats.

; RUN: opt -passes='cg-section-func-comdat-creator' -S %s | FileCheck %s

; CHECK: $f = comdat noduplicates
; CHECK: $g = comdat any
$g = comdat any

; CHECK: define dso_local void @f() comdat {
define dso_local void @f() {
entry:
  ret void
}

; CHECK: define dso_local void @g() comdat {
define dso_local void @g() comdat {
entry:
  ret void
}

