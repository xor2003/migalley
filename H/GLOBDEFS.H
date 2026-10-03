#if !defined(GLOBDEFS_H_INCLUDED)
#define GLOBDEFS_H_INCLUDED

#if 0 // RERUN WINDOWS definitions - commented out for Linux port
#undef	ON_MESSAGE
#define ON_MESSAGE(message, memberFxn) \
	{ message, 0, 0, 0, AfxSig_lwl, \
		(AFX_PMSG)(AFX_PMSGW)(LRESULT (AFX_MSG_CALL CWnd::*)(WPARAM, LPARAM))&MSG2_##memberFxn },
#define ON_MESSAGE_CLASS(msg,type,routine)\
	{ msg, 0, 0, 0, AfxSig_lwl, \
		(AFX_PMSG)(AFX_PMSGW)(LRESULT (AFX_MSG_CALL CWnd::*)(WPARAM, LPARAM))(LRESULT (AFX_MSG_CALL type::*)(WPARAM, LPARAM))&MSG2_##routine },
#define MSG2_0(memberFxn)	LRESULT MSG2_##memberFxn(int,int) {return (int)memberFxn();}
#define MSG2_1(memberFxn)	LRESULT MSG2_##memberFxn(int a,int) {return (int)memberFxn(a);}
#define MSG2_2(memberFxn)	LRESULT MSG2_##memberFxn(int a,int b) {return (int)memberFxn(a,b);}
#define MSG2_0v(memberFxn)	LRESULT MSG2_##memberFxn(int,int) {memberFxn();return 0;}
#define MSG2_1v(memberFxn)	LRESULT MSG2_##memberFxn(int a,int) {memberFxn(a);return 0;}
#define MSG2_2v(memberFxn)	LRESULT MSG2_##memberFxn(int a,int b) {memberFxn(a,b);return 0;}
#define MSG2(memberFxn)		LRESULT MSG2_##memberFxn(int a,int b)

#else
#undef ON_MESSAGE
#define ON_MESSAGE(message, memberFxn) \
    { message, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(LRESULT (CWnd::*)(WPARAM, LPARAM))&thisClass::MSG2_##memberFxn },

#define ON_MESSAGE_CLASS(msg, type, func) \
    { msg, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(LRESULT (CWnd::*)(WPARAM, LPARAM))&type::MSG2_##func },

// Rowan message wrapper stubs — preserve function signatures and provide bodies
#define MSG2(memberFxn)        LRESULT MSG2_##memberFxn(int a, int b)
#define MSG2_0(memberFxn)      LRESULT MSG2_##memberFxn(int, int) { return (LRESULT)memberFxn(); }
#define MSG2_1(memberFxn)      LRESULT MSG2_##memberFxn(int a, int) { return (LRESULT)memberFxn(a); }
#define MSG2_2(memberFxn)      LRESULT MSG2_##memberFxn(int a, int b) { return (LRESULT)memberFxn(a, b); }
#define MSG2_0v(memberFxn)     LRESULT MSG2_##memberFxn(int, int) { memberFxn(); return 0; }
#define MSG2_1v(memberFxn)     LRESULT MSG2_##memberFxn(int a, int) { memberFxn(a); return 0; }
#define MSG2_2v(memberFxn)     LRESULT MSG2_##memberFxn(int a, int b) { memberFxn(a, b); return 0; }
#endif // RERUN WINDOWS definitions - commented out

#endif