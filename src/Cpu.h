#ifndef _CPU_H
#define _CPU_H

class Memory;

class Cpu
{
public:
	Cpu();
	~Cpu();

	void Reset();
	bool Step();

	Memory * AttachedMemory;

	bool Running;
	long long Cycle;

	unsigned short InstructionPC();

	void RequestIrq(int sourceFlag);
	void UnrequestIrq(int sourceFlag);

protected:

	int RequestedInterrupts;
	bool HandleInterrupt;

	unsigned short SavedPC; // Save the PC for the instruction currently being executed.

	unsigned short PC; // Program counter
	unsigned char S; // Stack pointer
	unsigned char P; // Processor status
	unsigned char A; // Accumulator
	unsigned char X; // Index register X
	unsigned char Y; // Index register Y

	unsigned char Load(unsigned short Address);
	unsigned short Load16(unsigned short Address);

	void CheckHandleInterrupt();

	void SetResultFlags(unsigned char Result);

	unsigned char Add(unsigned char Add1, unsigned char Add2, int Carry);
	unsigned char Sub(unsigned char Sub1, unsigned char Sub2, int Carry);

	void SetFlag(unsigned char Flag);
	void ClearFlag(unsigned char Flag);

	unsigned char Low(unsigned short Reg);
	unsigned char High(unsigned short Reg);
	void SetLow(unsigned short & Reg, unsigned char NewLow);
	void SetHigh(unsigned short & Reg, unsigned char NewHigh);

	void Push(unsigned char Value);
	unsigned char Pop();

	unsigned char LoadInstructionByte();
	unsigned short LoadInstructionShort();

	// Bit flags
	const unsigned char NFlag = 0x80; // Negative. Set to the top bit of the result of an operation.
	const unsigned char VFlag = 0x40; // Overflow. Overflow is when the carry into the top bit != the carry out of the top bit (occurs when the resulting math operation wraps around)
	const unsigned char OneFlag = 0x20;// 0x20 flag is always 1
	const unsigned char BFlag = 0x10; // Break. Always set except when the P register is being pushed on stack to service a hardware interrupt (to distiguish from BRK instruction interrupts)
	const unsigned char DFlag = 0x08; // Decimal Mode. When 1, add/sub will use BCD mode. Annoying to emulate :)
	const unsigned char IFlag = 0x04; // Interrupt Disable. Set to 1 to prevent hardware interrupts.
	const unsigned char ZFlag = 0x02; // Zero. Set to 1 only when the result of the last operation was zero
	const unsigned char CFlag = 0x01; // Carry. The raw carry out of add/sub operations, or the ejected bit in bit shifts.

};

#endif