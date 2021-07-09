; Tests that we store the type identifiers in .callgraph section of the binary.

; RUN: llc --call-graph-section -filetype=obj -o - < %s | \
; RUN: llvm-readelf -x .callgraph - | FileCheck %s

; CHECK: Hex dump of section '.callgraph':

target triple = "x86_64-unknown-linux-gnu"

$foo = comdat noduplicates

$bar = comdat noduplicates

$baz = comdat noduplicates

$main = comdat noduplicates

define dso_local void @foo() #0 comdat !type !4 {
entry:
  ret void
}

define dso_local i32 @bar(i8 signext %a) #0 comdat !type !5 {
entry:
  %a.addr = alloca i8, align 1
  store i8 %a, i8* %a.addr, align 1
  ret i32 0
}

define dso_local i32* @baz(i8* %a) #0 comdat !type !6 {
entry:
  %a.addr = alloca i8*, align 8
  store i8* %a, i8** %a.addr, align 8
  ret i32* null
}

define dso_local i32 @main() #0 comdat !type !7 {
entry:
  %retval = alloca i32, align 4
  %fp_foo = alloca void (...)*, align 8
  %a = alloca i8, align 1
  %fp_bar = alloca i32 (i8)*, align 8
  %fp_baz = alloca i32* (i8*)*, align 8
  store i32 0, i32* %retval, align 4
  store void (...)* bitcast (void ()* @foo to void (...)*), void (...)** %fp_foo, align 8
  %0 = load void (...)*, void (...)** %fp_foo, align 8
  call void (...) %0(), !type !8
  store i32 (i8)* @bar, i32 (i8)** %fp_bar, align 8
  %1 = load i32 (i8)*, i32 (i8)** %fp_bar, align 8
  %2 = load i8, i8* %a, align 1
  %call = call i32 %1(i8 signext %2), !type !9
  store i32* (i8*)* @baz, i32* (i8*)** %fp_baz, align 8
  %3 = load i32* (i8*)*, i32* (i8*)** %fp_baz, align 8
  %call1 = call i32* %3(i8* %a), !type !10
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}

; Check that the numeric type id (md5 hash) for the below type ids are emitted
; to the callgraph section.

; CHECK-DAG: 2444f731 f5eecb3e
!4 = !{i64 0, !"_ZTSFvE.generalized"}

; CHECK-DAG: 5486bc59 814b8e30
!5 = !{i64 0, !"_ZTSFicE.generalized"}

; CHECK-DAG: 7ade6814 f897fd77
!6 = !{i64 0, !"_ZTSFPvS_E.generalized"}

; CHECK-DAG: caaf769a 600968fa
!7 = !{i64 0, !"_ZTSFiE.generalized"}

; CHECK-DAG: 2444f731 f5eecb3e
!8 = !{!"_ZTSFvE.generalized"}

; CHECK-DAG: 5486bc59 814b8e30
!9 = !{!"_ZTSFicE.generalized"}

; CHECK-DAG: 7ade6814 f897fd77
!10 = !{!"_ZTSFPvS_E.generalized"}

