#ifndef AU_GFX_MACRO_H
#define AU_GFX_MACRO_H

#include <aurora/window.h>
#include <aurora/gfx.h>
#include <iostream>

#define EXPAND(x) x

#define FOR_EACH_ARG(DO, ...) \
  EXPAND(FOR_EACH_ARG_IMPL(__VA_ARGS__, 63, 62, 61, 60, \
  59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
  49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
  39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
  29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
  19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
  9, 8, 7, 6, 5, 4, 3, 2, 1, 0)(DO, __VA_ARGS__))

#define FOR_EACH_ARG_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
  _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
  _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
  _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
  _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
  _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, \
  _61, _62, _63, N, ...) EACHARG ## N

#define EACHARG64(DO, ARG, ...) DO(63, ARG) EXPAND(EACHARG63(DO, __VA_ARGS__))
#define EACHARG63(DO, ARG, ...) DO(62, ARG) EXPAND(EACHARG62(DO, __VA_ARGS__))
#define EACHARG62(DO, ARG, ...) DO(61, ARG) EXPAND(EACHARG61(DO, __VA_ARGS__))
#define EACHARG61(DO, ARG, ...) DO(60, ARG) EXPAND(EACHARG60(DO, __VA_ARGS__))
#define EACHARG60(DO, ARG, ...) DO(59, ARG) EXPAND(EACHARG59(DO, __VA_ARGS__))
#define EACHARG59(DO, ARG, ...) DO(58, ARG) EXPAND(EACHARG58(DO, __VA_ARGS__))
#define EACHARG58(DO, ARG, ...) DO(57, ARG) EXPAND(EACHARG57(DO, __VA_ARGS__))
#define EACHARG57(DO, ARG, ...) DO(56, ARG) EXPAND(EACHARG56(DO, __VA_ARGS__))
#define EACHARG56(DO, ARG, ...) DO(55, ARG) EXPAND(EACHARG55(DO, __VA_ARGS__))
#define EACHARG55(DO, ARG, ...) DO(54, ARG) EXPAND(EACHARG54(DO, __VA_ARGS__))
#define EACHARG54(DO, ARG, ...) DO(53, ARG) EXPAND(EACHARG53(DO, __VA_ARGS__))
#define EACHARG53(DO, ARG, ...) DO(52, ARG) EXPAND(EACHARG52(DO, __VA_ARGS__))
#define EACHARG52(DO, ARG, ...) DO(51, ARG) EXPAND(EACHARG51(DO, __VA_ARGS__))
#define EACHARG51(DO, ARG, ...) DO(50, ARG) EXPAND(EACHARG50(DO, __VA_ARGS__))
#define EACHARG50(DO, ARG, ...) DO(49, ARG) EXPAND(EACHARG49(DO, __VA_ARGS__))
#define EACHARG49(DO, ARG, ...) DO(48, ARG) EXPAND(EACHARG48(DO, __VA_ARGS__))
#define EACHARG48(DO, ARG, ...) DO(47, ARG) EXPAND(EACHARG47(DO, __VA_ARGS__))
#define EACHARG47(DO, ARG, ...) DO(46, ARG) EXPAND(EACHARG46(DO, __VA_ARGS__))
#define EACHARG46(DO, ARG, ...) DO(45, ARG) EXPAND(EACHARG45(DO, __VA_ARGS__))
#define EACHARG45(DO, ARG, ...) DO(44, ARG) EXPAND(EACHARG44(DO, __VA_ARGS__))
#define EACHARG44(DO, ARG, ...) DO(43, ARG) EXPAND(EACHARG43(DO, __VA_ARGS__))
#define EACHARG43(DO, ARG, ...) DO(42, ARG) EXPAND(EACHARG42(DO, __VA_ARGS__))
#define EACHARG42(DO, ARG, ...) DO(41, ARG) EXPAND(EACHARG41(DO, __VA_ARGS__))
#define EACHARG41(DO, ARG, ...) DO(40, ARG) EXPAND(EACHARG40(DO, __VA_ARGS__))
#define EACHARG40(DO, ARG, ...) DO(39, ARG) EXPAND(EACHARG39(DO, __VA_ARGS__))
#define EACHARG39(DO, ARG, ...) DO(38, ARG) EXPAND(EACHARG38(DO, __VA_ARGS__))
#define EACHARG38(DO, ARG, ...) DO(37, ARG) EXPAND(EACHARG37(DO, __VA_ARGS__))
#define EACHARG37(DO, ARG, ...) DO(36, ARG) EXPAND(EACHARG36(DO, __VA_ARGS__))
#define EACHARG36(DO, ARG, ...) DO(35, ARG) EXPAND(EACHARG35(DO, __VA_ARGS__))
#define EACHARG35(DO, ARG, ...) DO(34, ARG) EXPAND(EACHARG34(DO, __VA_ARGS__))
#define EACHARG34(DO, ARG, ...) DO(33, ARG) EXPAND(EACHARG33(DO, __VA_ARGS__))
#define EACHARG33(DO, ARG, ...) DO(32, ARG) EXPAND(EACHARG32(DO, __VA_ARGS__))
#define EACHARG32(DO, ARG, ...) DO(31, ARG) EXPAND(EACHARG31(DO, __VA_ARGS__))
#define EACHARG31(DO, ARG, ...) DO(30, ARG) EXPAND(EACHARG30(DO, __VA_ARGS__))
#define EACHARG30(DO, ARG, ...) DO(29, ARG) EXPAND(EACHARG29(DO, __VA_ARGS__))
#define EACHARG29(DO, ARG, ...) DO(28, ARG) EXPAND(EACHARG28(DO, __VA_ARGS__))
#define EACHARG28(DO, ARG, ...) DO(27, ARG) EXPAND(EACHARG27(DO, __VA_ARGS__))
#define EACHARG27(DO, ARG, ...) DO(26, ARG) EXPAND(EACHARG26(DO, __VA_ARGS__))
#define EACHARG26(DO, ARG, ...) DO(25, ARG) EXPAND(EACHARG25(DO, __VA_ARGS__))
#define EACHARG25(DO, ARG, ...) DO(24, ARG) EXPAND(EACHARG24(DO, __VA_ARGS__))
#define EACHARG24(DO, ARG, ...) DO(23, ARG) EXPAND(EACHARG23(DO, __VA_ARGS__))
#define EACHARG23(DO, ARG, ...) DO(22, ARG) EXPAND(EACHARG22(DO, __VA_ARGS__))
#define EACHARG22(DO, ARG, ...) DO(21, ARG) EXPAND(EACHARG21(DO, __VA_ARGS__))
#define EACHARG21(DO, ARG, ...) DO(20, ARG) EXPAND(EACHARG20(DO, __VA_ARGS__))
#define EACHARG20(DO, ARG, ...) DO(19, ARG) EXPAND(EACHARG19(DO, __VA_ARGS__))
#define EACHARG19(DO, ARG, ...) DO(18, ARG) EXPAND(EACHARG18(DO, __VA_ARGS__))
#define EACHARG18(DO, ARG, ...) DO(17, ARG) EXPAND(EACHARG17(DO, __VA_ARGS__))
#define EACHARG17(DO, ARG, ...) DO(16, ARG) EXPAND(EACHARG16(DO, __VA_ARGS__))
#define EACHARG16(DO, ARG, ...) DO(15, ARG) EXPAND(EACHARG15(DO, __VA_ARGS__))
#define EACHARG15(DO, ARG, ...) DO(14, ARG) EXPAND(EACHARG14(DO, __VA_ARGS__))
#define EACHARG14(DO, ARG, ...) DO(13, ARG) EXPAND(EACHARG13(DO, __VA_ARGS__))
#define EACHARG13(DO, ARG, ...) DO(12, ARG) EXPAND(EACHARG12(DO, __VA_ARGS__))
#define EACHARG12(DO, ARG, ...) DO(11, ARG) EXPAND(EACHARG11(DO, __VA_ARGS__))
#define EACHARG11(DO, ARG, ...) DO(10, ARG) EXPAND(EACHARG10(DO, __VA_ARGS__))
#define EACHARG10(DO, ARG, ...) DO(9, ARG) EXPAND(EACHARG9(DO, __VA_ARGS__))
#define EACHARG9(DO, ARG, ...) DO(8, ARG) EXPAND(EACHARG8(DO, __VA_ARGS__))
#define EACHARG8(DO, ARG, ...) DO(7, ARG) EXPAND(EACHARG7(DO, __VA_ARGS__))
#define EACHARG7(DO, ARG, ...) DO(6, ARG) EXPAND(EACHARG6(DO, __VA_ARGS__))
#define EACHARG6(DO, ARG, ...) DO(5, ARG) EXPAND(EACHARG5(DO, __VA_ARGS__))
#define EACHARG5(DO, ARG, ...) DO(4, ARG) EXPAND(EACHARG4(DO, __VA_ARGS__))
#define EACHARG4(DO, ARG, ...) DO(3, ARG) EXPAND(EACHARG3(DO, __VA_ARGS__))
#define EACHARG3(DO, ARG, ...) DO(2, ARG) EXPAND(EACHARG2(DO, __VA_ARGS__))
#define EACHARG2(DO, ARG, ...) DO(1, ARG) EXPAND(EACHARG1(DO, __VA_ARGS__))
#define EACHARG1(DO, ARG, ...) DO(0, ARG)

#define LAYER_MESH_TYPE_IMPL(NAME) \
    struct NAME ## _Vertex \
    { \
    }; \
    class NAME ## _Mesh \
    { \
    public: \
     \
    };

#define LAYER_MESH_TYPE(I, ARG) \
    LAYER_MESH_TYPE_IMPL ARG

#define RENDER_FUNC_IMPL(NAME) \
    void NAME ## _Render() \
    { \
        std::cout << #NAME << " layer being rendered." << std::endl; \
    }

#define RENDER_FUNC(I, ARG) \
    RENDER_FUNC_IMPL ARG

#define RENDER_CALL_IMPL(NAME) \
    NAME ## _Render();

#define RENDER_CALL(I, ARG) \
    RENDER_CALL_IMPL ARG

#define AU_GFX_DEFINE_RENDER_SYSTEM(...) \
    Au::GFX::Device gfxDevice; \
    FOR_EACH_ARG(LAYER_MESH_TYPE, __VA_ARGS__) \
    FOR_EACH_ARG(RENDER_FUNC, __VA_ARGS__) \
    bool AuGFXInit(Au::Window* window) \
    { \
        gfxDevice.Init(*window); \
        return true; \
    } \
    void AuGFXCleanup() \
    { \
         \
    } \
    void AuGFXRender() \
    { \
        gfxDevice.Clear(); \
        FOR_EACH_ARG(RENDER_CALL, __VA_ARGS__) \
        gfxDevice.SwapBuffers(); \
    }

#endif
