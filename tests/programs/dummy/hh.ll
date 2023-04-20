define dso_local void @check(i8 signext %0) #0 {
  %2 = alloca i8, align 1
  store i8 %0, i8* %2, align 1
  %3 = load i8, i8* %2, align 1
  %4 = sext i8 %3 to i32
  %5 = icmp sgt i32 %4, 100
  br i1 %5, label %6, label %7

6:                                                ; preds = %1
  call void @assert(i8 signext 0)
  br label %7

7:                                                ; preds = %6, %1
  ret void
}

declare void @assert(i8 signext) #1

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local void @main() #0 {
  %1 = alloca i8, align 1
  %2 = call signext i8 () @k()
  store i8 %2, i8* %1, align 1
  %3 = load i8, i8* %1, align 1
  call void @check(i8 signext %3)
  ret void
}

declare signext i8 @k() #1