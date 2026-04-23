
; Function Attrs: nounwind uwtable
define i32 @main() #0 {
pred:
	br label %blk				

blk:				
	%x = phi i32 [ %z, %blk ], [ 0, %pred ]
	%z = phi i32 [ %x, %blk ], [ 1, %pred ]
	%b = icmp eq i32 %x, %z
	br i1 %b, label  %succ, label %blk
succ:
        call void @assert(i8 noundef signext 0)
	ret i32 0
}

declare void @assert(i8 noundef signext)

