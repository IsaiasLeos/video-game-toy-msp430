	.local musicState
	.comm musicState,2,2
	.text   	 
JT:
	.word OPTION0            
	.word OPTION1
	.word OPTION2
	.word OPTION3
	.word OPTION4
	.word OPTION5
	.word OPTION6
	.word OPTION7
	.word OPTION8
	.word OPTION9
	.word OPTION10
	.word OPTION11
	.word OPTION12
	.word OPTION13
	.word OPTION14
	.word OPTION15
	.word OPTION16
	.word DEFAULT
	.global playMusic
playMusic:
	CMP #17, &musicState
	JNC DEFAULT     
	MOV &musicState, R13
	ADD R13, R13
	MOV JT(R13), R0
OPTION0:
	MOV    #4000, R12
	CALL    #buzzer_set_period
	MOV    #1, &musicState
	JMP END
OPTION1:
	MOV    #2000, R12
	CALL    #buzzer_set_period
	MOV    #2, &musicState
	JMP END
OPTION2:
	MOV    #5000, R12
	CALL    #buzzer_set_period
	MOV    #3, &musicState
	JMP END
OPTION3:
	MOV    #4000, R12
	CALL    #buzzer_set_period
	MOV    #4, &musicState
	JMP END
OPTION4:
	MOV    #8000, R12
	CALL    #buzzer_set_period
	MOV    #5, &musicState
	JMP END
OPTION5:
	MOV    #6000, R12
	CALL    #buzzer_set_period
	MOV    #6, &musicState
OPTION6:
	MOV    #7000, R12
	CALL    #buzzer_set_period
	MOV    #7, &musicState
	JMP END
OPTION7:
	MOV    #8000, R12
	CALL    #buzzer_set_period
	MOV    #8, &musicState
	JMP END
OPTION8:
	MOV    #9000, R12
	CALL    #buzzer_set_period
	MOV    #9, &musicState
	JMP END
OPTION9:
	MOV    #4000, R12
	CALL    #buzzer_set_period
	MOV    #10, &musicState
	JMP END
OPTION10:
	MOV    #2000, R12
	CALL    #buzzer_set_period
	MOV    #11, &musicState
	JMP END
OPTION11:
	MOV    #7000, R12
	CALL    #buzzer_set_period
	MOV    #12, &musicState
	JMP END
OPTION12:
	MOV    #8000, R12
	CALL    #buzzer_set_period
	MOV    #13, &musicState
	JMP END
OPTION13:
	MOV    #7000, R12
	CALL    #buzzer_set_period
	MOV    #14, &musicState
OPTION14:
	MOV    #7000, R12
	CALL    #buzzer_set_period
	MOV    #15, &musicState
	JMP END
OPTION15:
	MOV    #6000, R12
	CALL    #buzzer_set_period
	MOV    #16, &musicState
	JMP END
OPTION16:
	MOV    #5000, R12
	CALL    #buzzer_set_period
	MOV    #0, &musicState
	JMP END
DEFAULT:
	MOV    #0, R12
	CALL    #buzzer_set_period
END:
	RET
	
