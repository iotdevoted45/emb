
#ifndef ALL_STEPPER_H
#define ALL_STEPPER_H

#include <Wire.h>
#include <MCP23017.h>

MCP23017 mcpU2 = MCP23017(MCP23017_U2);
MCP23017 mcpU3 = MCP23017(MCP23017_U3);
MCP23017 mcpU4 = MCP23017(MCP23017_U4);
MCP23017 mcpU5 = MCP23017(MCP23017_U5);
MCP23017 mcpU6 = MCP23017(MCP23017_U6);
MCP23017 mcpU7 = MCP23017(MCP23017_U7);

#define LOC_A (G_Loc[aa] == "A1") ||(G_Loc[aa] == "A2") ||(G_Loc[aa] == "A3") ||(G_Loc[aa] == "A4") ||(G_Loc[aa] == "A5")
#define LOC_B (G_Loc[aa] == "B1") ||(G_Loc[aa] == "B2") ||(G_Loc[aa] == "B3") ||(G_Loc[aa] == "B4") ||(G_Loc[aa] == "B5")
#define LOC_C (G_Loc[aa] == "C1") ||(G_Loc[aa] == "C2") ||(G_Loc[aa] == "C3") ||(G_Loc[aa] == "C4") ||(G_Loc[aa] == "C5")
#define LOC_D (G_Loc[aa] == "D1") ||(G_Loc[aa] == "D2") ||(G_Loc[aa] == "D3") ||(G_Loc[aa] == "D4") ||(G_Loc[aa] == "D5")
#define LOC_E (G_Loc[aa] == "E1") ||(G_Loc[aa] == "E2") ||(G_Loc[aa] == "E3") ||(G_Loc[aa] == "E4") ||(G_Loc[aa] == "E5")
  
char DELAY_ms = 50;
char DELAY_MS = 100;

void LINE_A_RVC(int AA, int BB){
    mcpU2.writeRegister(MCP23017Register::GPIO_B, AA);  //Reset port B
    delayMicroseconds(DELAY_ms);
    mcpU2.writeRegister(MCP23017Register::GPIO_B, BB);  //Reset port B
    delayMicroseconds(DELAY_MS);
}
void LINE_A_FRD(int CC, int DD){
    mcpU2.writeRegister(MCP23017Register::GPIO_B, CC);  //Reset port B  
    delayMicroseconds(DELAY_ms);
    mcpU2.writeRegister(MCP23017Register::GPIO_B, DD);  //Reset port B  
    delayMicroseconds(DELAY_MS);
}

///////////////////////////////////////////////////////////////////////
void LINE_B_RVC(int AA, int BB){
    mcpU2.writeRegister(MCP23017Register::GPIO_A, AA);  //Reset port B
    delayMicroseconds(DELAY_ms);
    mcpU2.writeRegister(MCP23017Register::GPIO_A, BB);  //Reset port B
    delayMicroseconds(DELAY_MS);
}
void LINE_B_FRD(int CC, int DD){
    mcpU2.writeRegister(MCP23017Register::GPIO_A, CC);  //Reset port B  
    delayMicroseconds(DELAY_ms);
    mcpU2.writeRegister(MCP23017Register::GPIO_A, DD);  //Reset port B  
    delayMicroseconds(DELAY_MS);
}

///////////////////////////////////////////////////////////////////////
void LINE_C_RVC(int AA, int BB){

    mcpU3.writeRegister(MCP23017Register::GPIO_B, AA);  //Reset port B
    delayMicroseconds(DELAY_ms);
    mcpU3.writeRegister(MCP23017Register::GPIO_B, BB);  //Reset port B
    delayMicroseconds(DELAY_MS);
}
void LINE_C_FRD(int CC, int DD){
    mcpU3.writeRegister(MCP23017Register::GPIO_B, CC);  //Reset port B  
    delayMicroseconds(DELAY_ms);
    mcpU3.writeRegister(MCP23017Register::GPIO_B, DD);  //Reset port B  
    delayMicroseconds(DELAY_MS);
}

///////////////////////////////////////////////////////////////////////
void LINE_D_RVC(int AA, int BB){

    mcpU3.writeRegister(MCP23017Register::GPIO_A, AA);  //Reset port B
    delayMicroseconds(DELAY_ms);
    mcpU3.writeRegister(MCP23017Register::GPIO_A, BB);  //Reset port B
    delayMicroseconds(DELAY_MS);
}
void LINE_D_FRD(int CC, int DD){
    mcpU3.writeRegister(MCP23017Register::GPIO_A, CC);  //Reset port B  
    delayMicroseconds(DELAY_ms);
    mcpU3.writeRegister(MCP23017Register::GPIO_A, DD);  //Reset port B  
    delayMicroseconds(DELAY_MS);
}

///////////////////////////////////////////////////////////////////////////
void LINE_E_FRD(int AA, int BB){
    mcpU4.writeRegister(MCP23017Register::GPIO_B, AA);  //Reset port B
    delayMicroseconds(DELAY_ms);
    mcpU4.writeRegister(MCP23017Register::GPIO_B, BB);  //Reset port B
    delayMicroseconds(DELAY_MS);
}
void LINE_E_RVC(int CC, int DD){
    mcpU4.writeRegister(MCP23017Register::GPIO_B, CC);  //Reset port B  
    delayMicroseconds(DELAY_ms);
    mcpU4.writeRegister(MCP23017Register::GPIO_B, DD);  //Reset port B  
    delayMicroseconds(DELAY_MS);
}

///////////////////////////////////////////////////////////////////////////
void LINE_5_RVC(int AA, int BB){
    mcpU4.writeRegister(MCP23017Register::GPIO_A, AA);  //Reset port B
    delayMicroseconds(DELAY_ms);
    mcpU4.writeRegister(MCP23017Register::GPIO_A, BB);  //Reset port B
    delayMicroseconds(DELAY_MS);
}
void LINE_5_FRD(int CC, int DD){
    mcpU4.writeRegister(MCP23017Register::GPIO_A, CC);  //Reset port B  
    delayMicroseconds(DELAY_ms);
    mcpU4.writeRegister(MCP23017Register::GPIO_A, DD);  //Reset port B  
    delayMicroseconds(DELAY_MS);
}

///////////////////////////////////////////////////////////////////////////
void LINE_X_FRD(int AA, int BB){
    mcpU5.writeRegister(MCP23017Register::GPIO_B, AA);  //Reset port B
    delayMicroseconds(DELAY_ms);
    mcpU5.writeRegister(MCP23017Register::GPIO_B, BB);  //Reset port B
    delayMicroseconds(DELAY_MS);
}
void LINE_X_RVC(int CC, int DD){
    mcpU5.writeRegister(MCP23017Register::GPIO_B, CC);  //Reset port B  
    delayMicroseconds(DELAY_ms);
    mcpU5.writeRegister(MCP23017Register::GPIO_B, DD);  //Reset port B  
    delayMicroseconds(DELAY_MS);
}
///////////////////////////////////////////////////////////////////////
#endif