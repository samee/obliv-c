#include <obliv_types.h>

void obliv_float_add_circuit(OblivBit* dest, const OblivBit* op1, const OblivBit* op2)
{
    OblivBit __obliv_0;
    OblivBit __obliv_1;
    OblivBit __obliv_2;
    OblivBit __obliv_3;
    OblivBit __obliv_4;
    OblivBit __obliv_5;
    OblivBit __obliv_6;
    OblivBit __obliv_7;
    OblivBit __obliv_8;
    OblivBit __obliv_9;
    OblivBit __obliv_10;
    OblivBit __obliv_11;
    OblivBit __obliv_12;
    OblivBit __obliv_13;
    OblivBit __obliv_14;
    OblivBit __obliv_15;
    OblivBit __obliv_16;
    OblivBit __obliv_17;
    OblivBit __obliv_18;
    OblivBit __obliv_19;
    OblivBit __obliv_20;
    OblivBit __obliv_21;
    OblivBit __obliv_22;
    OblivBit __obliv_23;
    OblivBit __obliv_24;
    OblivBit __obliv_25;
    OblivBit __obliv_26;
    OblivBit __obliv_27;
    OblivBit __obliv_28;
    OblivBit __obliv_29;
    OblivBit __obliv_30;
    OblivBit __obliv_31;
    OblivBit __obliv_32;
    OblivBit __obliv_33;
    OblivBit __obliv_34;
    OblivBit __obliv_35;
    OblivBit __obliv_36;
    OblivBit __obliv_37;
    OblivBit __obliv_38;
    OblivBit __obliv_39;
    OblivBit __obliv_40;
    OblivBit __obliv_41;
    OblivBit __obliv_42;
    OblivBit __obliv_43;
    OblivBit __obliv_44;
    OblivBit __obliv_45;
    OblivBit __obliv_46;
    OblivBit __obliv_47;
    OblivBit __obliv_48;
    OblivBit __obliv_49;
    OblivBit __obliv_50;
    OblivBit __obliv_51;
    OblivBit __obliv_52;
    OblivBit __obliv_53;
    OblivBit __obliv_54;
    OblivBit __obliv_55;
    OblivBit __obliv_56;
    OblivBit __obliv_57;
    OblivBit __obliv_58;
    OblivBit __obliv_59;
    OblivBit __obliv_60;
    OblivBit __obliv_61;
    OblivBit __obliv_62;
    OblivBit __obliv_63;
    OblivBit __obliv_64;

    __obliv_c__copyBit(&__obliv_0, op1+0);
    __obliv_c__copyBit(&__obliv_1, op1+1);
    __obliv_c__copyBit(&__obliv_2, op1+2);
    __obliv_c__copyBit(&__obliv_3, op1+3);
    __obliv_c__copyBit(&__obliv_4, op1+4);
    __obliv_c__copyBit(&__obliv_5, op1+5);
    __obliv_c__copyBit(&__obliv_6, op1+6);
    __obliv_c__copyBit(&__obliv_7, op1+7);
    __obliv_c__copyBit(&__obliv_8, op1+8);
    __obliv_c__copyBit(&__obliv_9, op1+9);
    __obliv_c__copyBit(&__obliv_10, op1+10);
    __obliv_c__copyBit(&__obliv_11, op1+11);
    __obliv_c__copyBit(&__obliv_12, op1+12);
    __obliv_c__copyBit(&__obliv_13, op1+13);
    __obliv_c__copyBit(&__obliv_14, op1+14);
    __obliv_c__copyBit(&__obliv_15, op1+15);
    __obliv_c__copyBit(&__obliv_16, op1+16);
    __obliv_c__copyBit(&__obliv_17, op1+17);
    __obliv_c__copyBit(&__obliv_18, op1+18);
    __obliv_c__copyBit(&__obliv_19, op1+19);
    __obliv_c__copyBit(&__obliv_20, op1+20);
    __obliv_c__copyBit(&__obliv_21, op1+21);
    __obliv_c__copyBit(&__obliv_22, op1+22);
    __obliv_c__copyBit(&__obliv_23, op1+23);
    __obliv_c__copyBit(&__obliv_24, op1+24);
    __obliv_c__copyBit(&__obliv_25, op1+25);
    __obliv_c__copyBit(&__obliv_26, op1+26);
    __obliv_c__copyBit(&__obliv_27, op1+27);
    __obliv_c__copyBit(&__obliv_28, op1+28);
    __obliv_c__copyBit(&__obliv_29, op1+29);
    __obliv_c__copyBit(&__obliv_30, op1+30);
    __obliv_c__copyBit(&__obliv_31, op1+31);
    __obliv_c__copyBit(&__obliv_32, op2+0);
    __obliv_c__copyBit(&__obliv_33, op2+1);
    __obliv_c__copyBit(&__obliv_34, op2+2);
    __obliv_c__copyBit(&__obliv_35, op2+3);
    __obliv_c__copyBit(&__obliv_36, op2+4);
    __obliv_c__copyBit(&__obliv_37, op2+5);
    __obliv_c__copyBit(&__obliv_38, op2+6);
    __obliv_c__copyBit(&__obliv_39, op2+7);
    __obliv_c__copyBit(&__obliv_40, op2+8);
    __obliv_c__copyBit(&__obliv_41, op2+9);
    __obliv_c__copyBit(&__obliv_42, op2+10);
    __obliv_c__copyBit(&__obliv_43, op2+11);
    __obliv_c__copyBit(&__obliv_44, op2+12);
    __obliv_c__copyBit(&__obliv_45, op2+13);
    __obliv_c__copyBit(&__obliv_46, op2+14);
    __obliv_c__copyBit(&__obliv_47, op2+15);
    __obliv_c__copyBit(&__obliv_48, op2+16);
    __obliv_c__copyBit(&__obliv_49, op2+17);
    __obliv_c__copyBit(&__obliv_50, op2+18);
    __obliv_c__copyBit(&__obliv_51, op2+19);
    __obliv_c__copyBit(&__obliv_52, op2+20);
    __obliv_c__copyBit(&__obliv_53, op2+21);
    __obliv_c__copyBit(&__obliv_54, op2+22);
    __obliv_c__copyBit(&__obliv_55, op2+23);
    __obliv_c__copyBit(&__obliv_56, op2+24);
    __obliv_c__copyBit(&__obliv_57, op2+25);
    __obliv_c__copyBit(&__obliv_58, op2+26);
    __obliv_c__copyBit(&__obliv_59, op2+27);
    __obliv_c__copyBit(&__obliv_60, op2+28);
    __obliv_c__copyBit(&__obliv_61, op2+29);
    __obliv_c__copyBit(&__obliv_62, op2+30);
    __obliv_c__copyBit(&__obliv_63, op2+31);
    
    __obliv_c__setBitNot(&__obliv_31, &__obliv_31);
    __obliv_c__setBitAnd(&__obliv_31, &__obliv_31, &__obliv_63);
    
    __obliv_c__copyBit(dest+0, &__obliv_0);
    __obliv_c__copyBit(dest+1, &__obliv_1);
    __obliv_c__copyBit(dest+2, &__obliv_2);
    __obliv_c__copyBit(dest+3, &__obliv_3);
    __obliv_c__copyBit(dest+4, &__obliv_4);
    __obliv_c__copyBit(dest+5, &__obliv_5);
    __obliv_c__copyBit(dest+6, &__obliv_6);
    __obliv_c__copyBit(dest+7, &__obliv_7);
    __obliv_c__copyBit(dest+8, &__obliv_8);
    __obliv_c__copyBit(dest+9, &__obliv_9);
    __obliv_c__copyBit(dest+10, &__obliv_10);
    __obliv_c__copyBit(dest+11, &__obliv_11);
    __obliv_c__copyBit(dest+12, &__obliv_12);
    __obliv_c__copyBit(dest+13, &__obliv_13);
    __obliv_c__copyBit(dest+14, &__obliv_14);
    __obliv_c__copyBit(dest+15, &__obliv_15);
    __obliv_c__copyBit(dest+16, &__obliv_16);
    __obliv_c__copyBit(dest+17, &__obliv_17);
    __obliv_c__copyBit(dest+18, &__obliv_18);
    __obliv_c__copyBit(dest+19, &__obliv_19);
    __obliv_c__copyBit(dest+20, &__obliv_20);
    __obliv_c__copyBit(dest+21, &__obliv_21);
    __obliv_c__copyBit(dest+22, &__obliv_22);
    __obliv_c__copyBit(dest+23, &__obliv_23);
    __obliv_c__copyBit(dest+24, &__obliv_24);
    __obliv_c__copyBit(dest+25, &__obliv_25);
    __obliv_c__copyBit(dest+26, &__obliv_26);
    __obliv_c__copyBit(dest+27, &__obliv_27);
    __obliv_c__copyBit(dest+28, &__obliv_28);
    __obliv_c__copyBit(dest+29, &__obliv_29);
    __obliv_c__copyBit(dest+30, &__obliv_30);
    __obliv_c__copyBit(dest+31, &__obliv_31);
    
}