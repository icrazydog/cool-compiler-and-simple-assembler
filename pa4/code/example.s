# start of generated code
	.data
	.align	2
	.globl	class_nameTab
	.globl	Main_protObj
	.globl	Int_protObj
	.globl	String_protObj
	.globl	bool_const0
	.globl	bool_const1
	.globl	_int_tag
	.globl	_bool_tag
	.globl	_string_tag
_int_tag:
	.word	2
_bool_tag:
	.word	3
_string_tag:
	.word	4
	.globl	_MemMgr_INITIALIZER
_MemMgr_INITIALIZER:
	.word	_NoGC_Init
	.globl	_MemMgr_COLLECTOR
_MemMgr_COLLECTOR:
	.word	_NoGC_Collect
	.globl	_MemMgr_TEST
_MemMgr_TEST:
	.word	0
	.word	-1
str_const11:
	.word	4
	.word	5
	.word	String_dispTab
	.word	int_const0
	.byte	0	
	.align	2
	.word	-1
str_const10:
	.word	4
	.word	6
	.word	String_dispTab
	.word	int_const1
	.ascii	"Main"
	.byte	0	
	.align	2
	.word	-1
str_const9:
	.word	4
	.word	6
	.word	String_dispTab
	.word	int_const2
	.ascii	"String"
	.byte	0	
	.align	2
	.word	-1
str_const8:
	.word	4
	.word	6
	.word	String_dispTab
	.word	int_const1
	.ascii	"Bool"
	.byte	0	
	.align	2
	.word	-1
str_const7:
	.word	4
	.word	5
	.word	String_dispTab
	.word	int_const3
	.ascii	"Int"
	.byte	0	
	.align	2
	.word	-1
str_const6:
	.word	4
	.word	5
	.word	String_dispTab
	.word	int_const4
	.ascii	"IO"
	.byte	0	
	.align	2
	.word	-1
str_const5:
	.word	4
	.word	6
	.word	String_dispTab
	.word	int_const2
	.ascii	"Object"
	.byte	0	
	.align	2
	.word	-1
str_const4:
	.word	4
	.word	7
	.word	String_dispTab
	.word	int_const5
	.ascii	"_prim_slot"
	.byte	0	
	.align	2
	.word	-1
str_const3:
	.word	4
	.word	7
	.word	String_dispTab
	.word	int_const6
	.ascii	"SELF_TYPE"
	.byte	0	
	.align	2
	.word	-1
str_const2:
	.word	4
	.word	7
	.word	String_dispTab
	.word	int_const6
	.ascii	"_no_class"
	.byte	0	
	.align	2
	.word	-1
str_const1:
	.word	4
	.word	8
	.word	String_dispTab
	.word	int_const7
	.ascii	"<basic class>"
	.byte	0	
	.align	2
	.word	-1
str_const0:
	.word	4
	.word	7
	.word	String_dispTab
	.word	int_const5
	.ascii	"example.cl"
	.byte	0	
	.align	2
	.word	-1
int_const7:
	.word	2
	.word	4
	.word	Int_dispTab
	.word	13
	.word	-1
int_const6:
	.word	2
	.word	4
	.word	Int_dispTab
	.word	9
	.word	-1
int_const5:
	.word	2
	.word	4
	.word	Int_dispTab
	.word	10
	.word	-1
int_const4:
	.word	2
	.word	4
	.word	Int_dispTab
	.word	2
	.word	-1
int_const3:
	.word	2
	.word	4
	.word	Int_dispTab
	.word	3
	.word	-1
int_const2:
	.word	2
	.word	4
	.word	Int_dispTab
	.word	6
	.word	-1
int_const1:
	.word	2
	.word	4
	.word	Int_dispTab
	.word	4
	.word	-1
int_const0:
	.word	2
	.word	4
	.word	Int_dispTab
	.word	0
	.word	-1
bool_const0:
	.word	3
	.word	4
	.word	Bool_dispTab
	.word	0
	.word	-1
bool_const1:
	.word	3
	.word	4
	.word	Bool_dispTab
	.word	1
	.word	-1
Main_protObj:
	.word	5
	.word	3
	.word	Main_dispTab
	.word	-1
String_protObj:
	.word	4
	.word	5
	.word	String_dispTab
	.word	int_const0
	.word	0
	.word	-1
Bool_protObj:
	.word	3
	.word	4
	.word	Bool_dispTab
	.word	0
	.word	-1
Int_protObj:
	.word	2
	.word	4
	.word	Int_dispTab
	.word	0
	.word	-1
IO_protObj:
	.word	1
	.word	3
	.word	IO_dispTab
	.word	-1
Object_protObj:
	.word	0
	.word	3
	.word	Object_dispTab
class_nameTab:
	.word	str_const5
	.word	str_const6
	.word	str_const7
	.word	str_const8
	.word	str_const9
	.word	str_const10
class_objTab:
	.word	Main_protObj
	.word	Main_init
	.word	String_protObj
	.word	String_init
	.word	Bool_protObj
	.word	Bool_init
	.word	Int_protObj
	.word	Int_init
	.word	IO_protObj
	.word	IO_init
	.word	Object_protObj
	.word	Object_init
Main_dispTab:
	.word	Object.abort
	.word	Object.type_name
	.word	Object.copy
	.word	Main.main
String_dispTab:
	.word	Object.abort
	.word	Object.type_name
	.word	Object.copy
	.word	String.length
	.word	String.concat
	.word	String.substr
Bool_dispTab:
	.word	Object.abort
	.word	Object.type_name
	.word	Object.copy
Int_dispTab:
	.word	Object.abort
	.word	Object.type_name
	.word	Object.copy
IO_dispTab:
	.word	Object.abort
	.word	Object.type_name
	.word	Object.copy
	.word	IO.out_string
	.word	IO.out_int
	.word	IO.in_string
	.word	IO.in_int
Object_dispTab:
	.word	Object.abort
	.word	Object.type_name
	.word	Object.copy
	.globl	heap_start
heap_start:
	.word	0
	.text
	.globl	Main_init
	.globl	Int_init
	.globl	String_init
	.globl	Bool_init
	.globl	Main.main
Main_init:
	sw	$ra 0($sp)
	move	$fp $sp
	addiu	$sp $sp -4
	move	$s0 $a0
	addiu	$sp $sp -8
	sw	$s0 8($sp)
	sw	$fp 4($sp)
	jal	Object_init
	lw	$s0 8($sp)
	lw	$fp 4($sp)
	addiu	$sp $sp 8
	lw	$ra 0($fp)
	addiu	$sp $sp 4
	move	$a0 $s0
	jr	$ra	
String_init:
	sw	$ra 0($sp)
	move	$fp $sp
	addiu	$sp $sp -4
	move	$s0 $a0
	addiu	$sp $sp -8
	sw	$s0 8($sp)
	sw	$fp 4($sp)
	jal	Object_init
	lw	$s0 8($sp)
	lw	$fp 4($sp)
	addiu	$sp $sp 8
	lw	$ra 0($fp)
	addiu	$sp $sp 4
	move	$a0 $s0
	jr	$ra	
Bool_init:
	sw	$ra 0($sp)
	move	$fp $sp
	addiu	$sp $sp -4
	move	$s0 $a0
	addiu	$sp $sp -8
	sw	$s0 8($sp)
	sw	$fp 4($sp)
	jal	Object_init
	lw	$s0 8($sp)
	lw	$fp 4($sp)
	addiu	$sp $sp 8
	lw	$ra 0($fp)
	addiu	$sp $sp 4
	move	$a0 $s0
	jr	$ra	
Int_init:
	sw	$ra 0($sp)
	move	$fp $sp
	addiu	$sp $sp -4
	move	$s0 $a0
	addiu	$sp $sp -8
	sw	$s0 8($sp)
	sw	$fp 4($sp)
	jal	Object_init
	lw	$s0 8($sp)
	lw	$fp 4($sp)
	addiu	$sp $sp 8
	lw	$ra 0($fp)
	addiu	$sp $sp 4
	move	$a0 $s0
	jr	$ra	
IO_init:
	sw	$ra 0($sp)
	move	$fp $sp
	addiu	$sp $sp -4
	move	$s0 $a0
	addiu	$sp $sp -8
	sw	$s0 8($sp)
	sw	$fp 4($sp)
	jal	Object_init
	lw	$s0 8($sp)
	lw	$fp 4($sp)
	addiu	$sp $sp 8
	lw	$ra 0($fp)
	addiu	$sp $sp 4
	move	$a0 $s0
	jr	$ra	
Object_init:
	sw	$ra 0($sp)
	move	$fp $sp
	addiu	$sp $sp -4
	move	$s0 $a0
	lw	$ra 0($fp)
	addiu	$sp $sp 4
	move	$a0 $s0
	jr	$ra	
Main.main:
	sw	$ra 0($sp)
	move	$fp $sp
	addiu	$sp $sp -4
	move	$s0 $a0
	addiu	$sp $sp 0
	la	$a0 int_const0
	addiu	$sp $sp 0
	lw	$ra 0($fp)
	addiu	$sp $sp 4
	addiu	$sp $sp 0
	jr	$ra	

# end of generated code
