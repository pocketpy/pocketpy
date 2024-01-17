#include "pocketpy/easing.h"

namespace pkpy{
// https://easings.net/

const double kPi = 3.1415926545;

static double easeLinear( double x ) {
    return x;
}

static double easeInSine( double x ) {
    return 1.0 - std::cos( x * kPi / 2 );
}

static double easeOutSine( double x ) {
	return std::sin( x * kPi / 2 );
}

static double easeInOutSine( double x ) {
	return -( std::cos( kPi * x ) - 1 ) / 2;
}

static double easeInQuad( double x ) {
    return x * x;
}

static double easeOutQuad( double x ) {
    return 1 - std::pow( 1 - x, 2 );
}

static double easeInOutQuad( double x ) {
    if( x < 0.5 ) {
        return 2 * x * x;
    } else {
        return 1 - std::pow( -2 * x + 2, 2 ) / 2;
    }
}

static double easeInCubic( double x ) {
    return x * x * x;
}

static double easeOutCubic( double x ) {
    return 1 - std::pow( 1 - x, 3 );
}

static double easeInOutCubic( double x ) {
    if( x < 0.5 ) {
        return 4 * x * x * x;
    } else {
        return 1 - std::pow( -2 * x + 2, 3 ) / 2;
    }
}

static double easeInQuart( double x ) {
    return std::pow( x, 4 );
}

static double easeOutQuart( double x ) {
    return 1 - std::pow( 1 - x, 4 );
}

static double easeInOutQuart( double x ) {
    if( x < 0.5 ) {
        return 8 * std::pow( x, 4 );
    } else {
        return 1 - std::pow( -2 * x + 2, 4 ) / 2;
    }
}

static double easeInQuint( double x ) {
    return std::pow( x, 5 );
}

static double easeOutQuint( double x ) {
    return 1 - std::pow( 1 - x, 5 );
}

static double easeInOutQuint( double x ) {
    if( x < 0.5 ) {
        return 16 * std::pow( x, 5 );
    } else {
        return 1 - std::pow( -2 * x + 2, 5 ) / 2;
    }
}

static double easeInExpo( double x ) {
    return x == 0 ? 0 : std::pow( 2, 10 * x - 10 );
}

static double easeOutExpo( double x ) {
    return x == 1 ? 1 : 1 - std::pow( 2, -10 * x );
}

static double easeInOutExpo( double x ) {
    if( x == 0 ) {
        return 0;
    } else if( x == 1 ) {
        return 1;
    } else if( x < 0.5 ) {
        return std::pow( 2, 20 * x - 10 ) / 2;
    } else {
        return (2 - std::pow( 2, -20 * x + 10 )) / 2;
    }
}

static double easeInCirc( double x ) {
    return 1 - std::sqrt( 1 - std::pow( x, 2 ) );
}

static double easeOutCirc( double x ) {
    return std::sqrt( 1 - std::pow( x - 1, 2 ) );
}

static double easeInOutCirc( double x ) {
    if( x < 0.5 ) {
        return (1 - std::sqrt( 1 - std::pow( 2 * x, 2 ) )) / 2;
    } else {
        return (std::sqrt( 1 - std::pow( -2 * x + 2, 2 ) ) + 1) / 2;
    }
}

static double easeInBack( double x ) {
    const double c1 = 1.70158;
    const double c3 = c1 + 1;
    return c3 * x * x * x - c1 * x * x;
}

static double easeOutBack( double x ) {
    const double c1 = 1.70158;
    const double c3 = c1 + 1;
    return 1 + c3 * std::pow( x - 1, 3 ) + c1 * std::pow( x - 1, 2 );
}

static double easeInOutBack( double x ) {
    const double c1 = 1.70158;
    const double c2 = c1 * 1.525;
    if( x < 0.5 ) {
        return (std::pow( 2 * x, 2 ) * ((c2 + 1) * 2 * x - c2)) / 2;
    } else {
        return (std::pow( 2 * x - 2, 2 ) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
    }
}

static double easeInElastic( double x ) {
    const double c4 = (2 * kPi) / 3;
    if( x == 0 ) {
        return 0;
    } else if( x == 1 ) {
        return 1;
    } else {
        return -std::pow( 2, 10 * x - 10 ) * std::sin( (x * 10 - 10.75) * c4 );
    }
}

static double easeOutElastic( double x ) {
    const double c4 = (2 * kPi) / 3;
    if( x == 0 ) {
        return 0;
    } else if( x == 1 ) {
        return 1;
    } else {
        return std::pow( 2, -10 * x ) * std::sin( (x * 10 - 0.75) * c4 ) + 1;
    }
}

static double easeInOutElastic( double x ) {
    const double c5 = (2 * kPi) / 4.5;
    if( x == 0 ) {
        return 0;
    } else if( x == 1 ) {
        return 1;
    } else if( x < 0.5 ) {
        return -(std::pow( 2, 20 * x - 10 ) * std::sin( (20 * x - 11.125) * c5 )) / 2;
    } else {
        return (std::pow( 2, -20 * x + 10 ) * std::sin( (20 * x - 11.125) * c5 )) / 2 + 1;
    }
}

static double easeOutBounce( double x ) {
    const double n1 = 7.5625;
    const double d1 = 2.75;
    if( x < 1 / d1 ) {
        return n1 * x * x;
    } else if( x < 2 / d1 ) {
        x -= 1.5 / d1;
        return n1 * x * x + 0.75;
    } else if( x < 2.5 / d1 ) {
        x -= 2.25 / d1;
        return n1 * x * x + 0.9375;
    } else {
        x -= 2.625 / d1;
        return n1 * x * x + 0.984375;
    }
}

static double easeInBounce( double x ) {
    return 1 - easeOutBounce(1 - x);
}

static double easeInOutBounce( double x ) {
    return x < 0.5
    ? (1 - easeOutBounce(1 - 2 * x)) / 2
    : (1 + easeOutBounce(2 * x - 1)) / 2;
}

void add_module_easing(VM* vm){
    PyObject* mod = vm->new_module("easing");

#define EASE(name)  \
    vm->bind_func<1>(mod, #name, [](VM* vm, ArgsView args){  \
        f64 t = CAST(f64, args[0]); \
        return VAR(ease##name(t));   \
    });

    EASE(Linear)
    EASE(InSine)
    EASE(OutSine)
    EASE(InOutSine)
    EASE(InQuad)
    EASE(OutQuad)
    EASE(InOutQuad)
    EASE(InCubic)
    EASE(OutCubic)
    EASE(InOutCubic)
    EASE(InQuart)
    EASE(OutQuart)
    EASE(InOutQuart)
    EASE(InQuint)
    EASE(OutQuint)
    EASE(InOutQuint)
    EASE(InExpo)
    EASE(OutExpo)
    EASE(InOutExpo)
    EASE(InCirc)
    EASE(OutCirc)
    EASE(InOutCirc)
    EASE(InBack)
    EASE(OutBack)
    EASE(InOutBack)
    EASE(InElastic)
    EASE(OutElastic)
    EASE(InOutElastic)
    EASE(InBounce)
    EASE(OutBounce)
    EASE(InOutBounce)

#undef EASE
}
}   // namespace pkpy