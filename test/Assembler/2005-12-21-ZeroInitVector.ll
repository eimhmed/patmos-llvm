; RUN: llvm-as < %s > /dev/null
; RUN: verify-uselistorder %s -preserve-bc-use-list-order -num-shuffles=5

define <4 x i32> @foo() {
        ret <4 x i32> zeroinitializer
}

