
declare void @assert(i8 signext)

define void @main () {
     %1 = insertvalue {i32, i32} {i32 1,i32 1}, i32 1, 0
     %2 = extractvalue {i32, i32} %1, 0
     %3 = icmp eq i32 1, %2
     %4 = sext i1 %3 to i8
     call void @assert(i8 %4)
     ret void       
}
