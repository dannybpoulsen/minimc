#include <vector>
#include <string>
#include <unordered_map>
#include "loaders/loader.hpp"
#include "model/analysis/find_location_defs.hpp"

const std::string ir = R"(
; ModuleID = 'main.c'
source_filename = "main.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 0, i32* %2, align 4
  store i32 0, i32* %3, align 4
  br label %4

4:                                                ; preds = %7, %0
  %5 = load i32, i32* %2, align 4
  %6 = icmp slt i32 %5, 5
  br i1 %6, label %7, label %10

7:                                                ; preds = %4
  %8 = load i32, i32* %2, align 4
  %9 = add nsw i32 %8, 1
  store i32 %9, i32* %2, align 4
  br label %4

10:                                               ; preds = %4
  %11 = load i32, i32* %1, align 4
  ret i32 %11
}

attributes #0 = { noinline nounwind optnone sspstrong uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{!"clang version 9.0.1 "}
)";

int main () {
  	//Load Program
  MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64> ();
  MiniMC::Model::ConstantFactory_ptr cfac = std::make_shared<MiniMC::Model::ConstantFactory64> (tfac);
  MiniMC::Loaders::LoadResult result = MiniMC::Loaders::loadFromString<MiniMC::Loaders::Type::LLVM> (ir, typename MiniMC::Loaders::OptionsLoad<MiniMC::Loaders::Type::LLVM>::Opt {.tfactory = tfac, .cfactory =cfac});
  
  result.program->addEntryPoint ("main");

  MiniMC::Model::Analysis::calculateDefs (*result.program->getFunction("main").get());
  
}
