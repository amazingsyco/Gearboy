#include "Processor.h"
#include "Memory.h"

Processor::Processor(Memory* pMemory)
{
    m_pMemory = pMemory;
}

Processor::~Processor()
{
}

void Processor::Reset()
{
    PC.SetValue(0x100);
    SP.SetValue(0xFFFE);
    AF.SetValue(0x01B0);
    BC.SetValue(0x0013);
    DE.SetValue(0x00D8);
    HL.SetValue(0x014D);
    m_pMemory->Reset();
}

void Processor::ClearAllFlags()
{
    SetFlag(FLAG_NONE);
}

void Processor::ToggleZeroFlagFromResult(u8 result)
{
    if (result == 0)
    {
        ToggleFlag(FLAG_ZERO);
    }
}

void Processor::SetFlag(u8 flag)
{
    AF.SetLow(flag);
}

void Processor::FlipFlag(u8 flag)
{
    AF.SetLow(AF.GetLow() ^ flag);
}

void Processor::ToggleFlag(u8 flag)
{
    AF.SetLow(AF.GetLow() | flag);
}

void Processor::UntoggleFlag(u8 flag)
{
    AF.SetLow(AF.GetLow() & (~flag));
}

bool Processor::IsSetFlag(u8 flag)
{
    return (AF.GetLow() & flag);
}

void Processor::StackPush(SixteenBitRegister* reg)
{
    SP.Decrement();
    m_pMemory->Write(SP.GetValue(), reg->GetHigh());
    SP.Decrement();
    m_pMemory->Write(SP.GetValue(), reg->GetLow());
}

void Processor::StackPop(SixteenBitRegister* reg)
{
    reg->SetLow(m_pMemory->Read(SP.GetValue()));
    SP.Increment();
    reg->SetHigh(m_pMemory->Read(SP.GetValue()));
    SP.Increment();
}

void Processor::InvalidOPCode()
{

}

void Processor::OPCodes_LD(EightBitRegister* reg1, u8 reg2)
{
    reg1->SetValue(reg2);
}

void Processor::OPCodes_LD(EightBitRegister* reg, u16 address)
{
    reg->SetValue(m_pMemory->Read(address));
}

void Processor::OPCodes_LD(u16 address, u8 reg)
{
    m_pMemory->Write(address, reg);
}

void Processor::OPCodes_OR(u8 number)
{
    u8 result = AF.GetHigh() | number;
    AF.SetHigh(result);
    ClearAllFlags();
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_XOR(u8 number)
{
    u8 result = AF.GetHigh() ^ number;
    AF.SetHigh(result);
    ClearAllFlags();
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_AND(u8 number)
{
    u8 result = AF.GetHigh() & number;
    AF.SetHigh(result);
    SetFlag(FLAG_HALF);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_CP(u8 number)
{
    SetFlag(FLAG_SUB);

    if (AF.GetHigh() < number)
    {
        ToggleFlag(FLAG_CARRY);
    }

    if (AF.GetHigh() == number)
    {
        ToggleFlag(FLAG_ZERO);
    }

    if (((AF.GetHigh() & 0x1F) - (number & 0x1F)) < 0x1F)
    {
        ToggleFlag(FLAG_HALF);
    }
}

void Processor::OPCodes_INC(EightBitRegister* reg)
{
    u8 result = reg->GetValue() + 1;
    reg->SetValue(result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleZeroFlagFromResult(result);
    if ((result & 0x0F) == 0x00)
    {
        ToggleFlag(FLAG_HALF);
    }
}

void Processor::OPCodes_INC_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue()) + 1;
    m_pMemory->Write(HL.GetValue(), result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleZeroFlagFromResult(result);
    if ((result & 0x0F) == 0x00)
    {
        ToggleFlag(FLAG_HALF);
    }
}

void Processor::OPCodes_DEC(EightBitRegister* reg)
{
    u8 result = reg->GetValue() - 1;
    reg->SetValue(result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleFlag(FLAG_SUB);
    ToggleZeroFlagFromResult(result);
    if ((result & 0x0F) == 0x0F)
    {
        ToggleFlag(FLAG_HALF);
    }
}

void Processor::OPCodes_DEC_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue()) - 1;
    m_pMemory->Write(HL.GetValue(), result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleFlag(FLAG_SUB);
    ToggleZeroFlagFromResult(result);
    if ((result & 0x0F) == 0x0F)
    {
        ToggleFlag(FLAG_HALF);
    }
}

void Processor::OPCodes_ADD(u8 number)
{
    int result = AF.GetHigh() + number;
    int carrybits = AF.GetHigh() ^ number ^ result;
    AF.SetHigh(static_cast<u8> (result));
    ClearAllFlags();
    ToggleZeroFlagFromResult(static_cast<u8> (result));
    if ((carrybits & 0x100) != 0)
    {
        ToggleFlag(FLAG_CARRY);
    }
    if ((carrybits & 0x10) != 0)
    {
        ToggleFlag(FLAG_HALF);
    }
}

void Processor::OPCodes_ADC(u8 number)
{
    int carry = IsSetFlag(FLAG_CARRY) ? 1 : 0;
    int result = AF.GetHigh() + number + carry;
    int carrybits = AF.GetHigh() ^ (number + carry) ^ result;
    AF.SetHigh(static_cast<u8> (result));
    ClearAllFlags();
    ToggleZeroFlagFromResult(static_cast<u8> (result));
    if ((carrybits & 0x100) != 0)
    {
        ToggleFlag(FLAG_CARRY);
    }
    if ((carrybits & 0x10) != 0)
    {
        ToggleFlag(FLAG_HALF);
    }
}

void Processor::OPCodes_SUB(u8 number)
{
    int result = AF.GetHigh() - number;
    int carrybits = AF.GetHigh() ^ number ^ result;
    AF.SetHigh(static_cast<u8> (result));
    SetFlag(FLAG_SUB);
    ToggleZeroFlagFromResult(static_cast<u8> (result));
    if ((carrybits & 0x100) != 0)
    {
        ToggleFlag(FLAG_CARRY);
    }
    if ((carrybits & 0x10) != 0)
    {
        ToggleFlag(FLAG_HALF);
    }
}

void Processor::OPCodes_SBC(u8 number)
{
    int carry = IsSetFlag(FLAG_CARRY) ? 1 : 0;
    int result = AF.GetHigh() - (number + carry);
    int carrybits = AF.GetHigh() ^ (number + carry) ^ result;
    AF.SetHigh(static_cast<u8> (result));
    SetFlag(FLAG_SUB);
    ToggleZeroFlagFromResult(static_cast<u8> (result));
    if ((carrybits & 0x100) != 0)
    {
        ToggleFlag(FLAG_CARRY);
    }
    if ((carrybits & 0x10) != 0)
    {
        ToggleFlag(FLAG_HALF);
    }
}

void Processor::OPCodes_ADD_HL(u16 number)
{
    int result = HL.GetValue() + number;
    IsSetFlag(FLAG_ZERO) ? SetFlag(FLAG_ZERO) : ClearAllFlags();
    if (result & 0x10000)
        ToggleFlag(FLAG_CARRY);
    if ((HL.GetValue() ^ number ^ (result & 0xFFFF)) & 0x1000)
        ToggleFlag(FLAG_HALF);
    HL.SetValue(static_cast<u16> (result));
}

void Processor::OPCodes_ADD_SP(s8 number)
{
    int result = SP.GetValue() + number;
    ClearAllFlags();
    if (number >= 0)
    {
        if (SP.GetValue() > result)
            ToggleFlag(FLAG_CARRY);
        if (((SP.GetValue() ^ number ^ result) & 0x1000) != 0)
            ToggleFlag(FLAG_HALF);
    }
    else
    {
        if (SP.GetValue() < result)
            ToggleFlag(FLAG_CARRY);
        if (((SP.GetValue() ^ number ^ result) & 0x1000) != 0)
            ToggleFlag(FLAG_HALF);
    }
    SP.SetValue(static_cast<u16> (result));
}

void Processor::OPCodes_SWAP_Register(EightBitRegister* reg)
{
    u8 low_half = reg->GetValue() & 0x0F;
    u8 high_half = (reg->GetValue() >> 4) & 0x0F;
    reg->SetValue((low_half << 4) + high_half);
    ClearAllFlags();
    ToggleZeroFlagFromResult(reg->GetValue());
}

void Processor::OPCodes_SWAP_HL()
{
    u8 number = m_pMemory->Read(HL.GetValue());
    u8 low_half = number & 0x0F;
    u8 high_half = (number >> 4) & 0x0F;
    number = (low_half << 4) + high_half;
    m_pMemory->Write(HL.GetValue(), number);
    ClearAllFlags();
    ToggleZeroFlagFromResult(number);
}

void Processor::OPCodes_SLA(EightBitRegister* reg)
{
    (reg->GetValue() & 0x80) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    u8 result = reg->GetValue() << 1;
    reg->SetValue(result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_SLA_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    (result & 0x80) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result <<= 1;
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_SRA(EightBitRegister* reg)
{
    u8 result = reg->GetValue();
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    if ((result & 0x80) != 0)
    {
        result >>= 1;
        result |= 0x80;
    }
    else
    {
        result >>= 1;
    }
    reg->SetValue(result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_SRA_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    if ((result & 0x80) != 0)
    {
        result >>= 1;
        result |= 0x80;
    }
    else
    {
        result >>= 1;
    }
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_SRL(EightBitRegister* reg)
{
    u8 result = reg->GetValue();
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    reg->SetValue(result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_SRL_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_RLC(EightBitRegister* reg)
{
    u8 result = reg->GetValue();
    if ((result & 0x80) != 0)
    {
        SetFlag(FLAG_CARRY);
        result <<= 1;
        result |= 0x1;
    }
    else
    {
        ClearAllFlags();
        result <<= 1;
    }
    reg->SetValue(result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_RLC_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    if ((result & 0x80) != 0)
    {
        SetFlag(FLAG_CARRY);
        result <<= 1;
        result |= 0x1;
    }
    else
    {
        ClearAllFlags();
        result <<= 1;
    }
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_RL(EightBitRegister* reg)
{
    u8 carry = IsSetFlag(FLAG_CARRY) ? 1 : 0;
    u8 result = reg->GetValue();
    ((result & 0x80) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result <<= 1;
    result |= carry;
    reg->SetValue(result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_RL_HL()
{
    u8 carry = IsSetFlag(FLAG_CARRY) ? 1 : 0;
    u8 result = m_pMemory->Read(HL.GetValue());
    ((result & 0x80) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result <<= 1;
    result |= carry;
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_RRC(EightBitRegister* reg)
{
    u8 result = reg->GetValue();
    if ((result & 0x01) != 0)
    {
        SetFlag(FLAG_CARRY);
        result >>= 1;
        result |= 0x80;
    }
    else
    {
        ClearAllFlags();
        result >>= 1;
    }
    reg->SetValue(result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_RRC_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    if ((result & 0x01) != 0)
    {
        SetFlag(FLAG_CARRY);
        result >>= 1;
        result |= 0x80;
    }
    else
    {
        ClearAllFlags();
        result >>= 1;
    }
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_RR(EightBitRegister* reg)
{
    u8 carry = IsSetFlag(FLAG_CARRY) ? 0x80 : 0x00;
    u8 result = reg->GetValue();
    ((result & 0x01) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    result |= carry;
    reg->SetValue(result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_RR_HL()
{
    u8 carry = IsSetFlag(FLAG_CARRY) ? 0x80 : 0x00;
    u8 result = m_pMemory->Read(HL.GetValue());
    ((result & 0x01) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    result |= carry;
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodes_BIT(EightBitRegister* reg, int bit)
{
    if (((reg->GetValue() >> bit) & 0x01) == 0)
        ToggleFlag(FLAG_ZERO);
    else
        UntoggleFlag(FLAG_ZERO);
    ToggleFlag(FLAG_HALF);
    UntoggleFlag(FLAG_SUB);
}

void Processor::OPCodes_BIT_HL(int bit)
{
    if (((m_pMemory->Read(HL.GetValue()) >> bit) & 0x01) == 0)
        ToggleFlag(FLAG_ZERO);
    else
        UntoggleFlag(FLAG_ZERO);
    ToggleFlag(FLAG_HALF);
    UntoggleFlag(FLAG_SUB);
}

void Processor::OPCodes_SET(EightBitRegister* reg, int bit)
{
    reg->SetValue(reg->GetValue() | (0x1 << bit));
}

void Processor::OPCodes_SET_HL(int bit)
{
    u8 result = m_pMemory->Read(HL.GetValue());
    result |= (0x1 << bit);
    m_pMemory->Write(HL.GetValue(), result);
}

void Processor::OPCodes_RES(EightBitRegister* reg, int bit)
{
    reg->SetValue(reg->GetValue() & (~(0x1 << bit)));
}

void Processor::OPCodes_RES_HL(int bit)
{
    u8 result = m_pMemory->Read(HL.GetValue());
    result &= ~(0x1 << bit);
    m_pMemory->Write(HL.GetValue(), result);
}

