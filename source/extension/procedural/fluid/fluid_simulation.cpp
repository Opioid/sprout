#include "fluid_simulation.hpp"
#include "base/memory/align.hpp"
#include "base/math/matrix3x3.inl"
#include "fluid_vorton.inl"

namespace procedural::fluid {

static uint32_t constexpr Num_vortons = 1024;

void compute_jacobian(Grid<float3x3>& jacobian , Grid<float3> const& vec, float3 const& extent);

Simulation::Simulation(int3 const& dimensions) noexcept
    : velocity_(dimensions),
      vortons_(memory::allocate_aligned<Vorton>(Num_vortons)),
      num_vortons_(Num_vortons) {}

Simulation::~Simulation() noexcept {
    memory::free_aligned(vortons_);
}

float3 Simulation::compute_velocity(float3 const& position) const noexcept {
    float3 velocity(0.f);

    for (uint32_t i = 0, len = num_vortons_; i < len; ++i) {
        velocity += vortons_[i].accumulate_velocity(position);
    }

    return velocity;
}

/*! \brief Compute Jacobian of a vector field

    \param jacobian - (output) UniformGrid of 3x3 matrix values.
                        The matrix is a vector of vectors.
                        Each component is a partial derivative with
                        respect to some direction:
                            j.a.b = d v.b / d a
                        where a and b are each one of {x,y,z}.
                        So j.x contains the partial derivatives with respect to x, etc.

    \param vec - UniformGrid of 3-vector values

*/
void compute_jacobian(Grid<float3x3>& jacobian , Grid<float3> const& vec, float3 const& extent) {
    const float3     spacing                 = extent / float3(vec.dimensions());
    // Avoid divide-by-zero when z size is effectively 0 (for 2D domains)
    const float3      reciprocalSpacing( 1.0f / spacing[0] , 1.0f / spacing[1] , spacing[2] > FLT_EPSILON ? 1.0f / spacing[2] : 0.0f ) ;
    const float3      halfReciprocalSpacing( 0.5f * reciprocalSpacing ) ;
    const int32_t  dims[3]                 = { vec.dimensions()[0] + 1, vec.dimensions()[1] + 1 , vec.dimensions()[2] + 1  } ;
    const int32_t  dimsMinus1[3]           = { vec.dimensions()[0] , vec.dimensions()[1] , vec.dimensions()[2] } ;
    const int32_t  numXY                   = dims[0] * dims[1] ;
    int32_t        index[3] ;

#define ASSIGN_Z_OFFSETS                                    \
    const unsigned offsetZM = numXY * ( index[2] - 1 ) ;    \
    const unsigned offsetZ0 = numXY *   index[2]       ;    \
    const unsigned offsetZP = numXY * ( index[2] + 1 ) ;

#define ASSIGN_YZ_OFFSETS                                                   \
    const unsigned offsetYMZ0 = dims[ 0 ] * ( index[1] - 1 ) + offsetZ0 ;   \
    const unsigned offsetY0Z0 = dims[ 0 ] *   index[1]       + offsetZ0 ;   \
    const unsigned offsetYPZ0 = dims[ 0 ] * ( index[1] + 1 ) + offsetZ0 ;   \
    const unsigned offsetY0ZM = dims[ 0 ] *   index[1]       + offsetZM ;   \
    const unsigned offsetY0ZP = dims[ 0 ] *   index[1]       + offsetZP ;

#define ASSIGN_XYZ_OFFSETS                                      \
    const unsigned offsetX0Y0Z0 = index[0]     + offsetY0Z0 ;   \
    const unsigned offsetXMY0Z0 = index[0] - 1 + offsetY0Z0 ;   \
    const unsigned offsetXPY0Z0 = index[0] + 1 + offsetY0Z0 ;   \
    const unsigned offsetX0YMZ0 = index[0]     + offsetYMZ0 ;   \
    const unsigned offsetX0YPZ0 = index[0]     + offsetYPZ0 ;   \
    const unsigned offsetX0Y0ZM = index[0]     + offsetY0ZM ;   \
    const unsigned offsetX0Y0ZP = index[0]     + offsetY0ZP ;

    // Compute derivatives for interior (i.e. away from boundaries).
    for( index[2] = 1 ; index[2] < dimsMinus1[2] ; ++ index[2] )
    {
        ASSIGN_Z_OFFSETS ;
        for( index[1] = 1 ; index[1] < dimsMinus1[1] ; ++ index[1] )
        {
            ASSIGN_YZ_OFFSETS ;
            for( index[0] = 1 ; index[0] < dimsMinus1[0] ; ++ index[0] )
            {
                ASSIGN_XYZ_OFFSETS ;

                float3x3 & rMatrix = jacobian.data()[ offsetX0Y0Z0 ] ;
                /* Compute d/dx */
                rMatrix.r[0] = halfReciprocalSpacing[0] * ( vec.at( offsetXPY0Z0 ) - vec.at( offsetXMY0Z0 ) )  ;
                /* Compute d/dy */
                rMatrix.r[1] = halfReciprocalSpacing[1] * ( vec.at( offsetX0YPZ0 ) - vec.at( offsetX0YMZ0 ) ) ;
                /* Compute d/dz */
                rMatrix.r[2] = halfReciprocalSpacing[2] * ( vec.at( offsetX0Y0ZP ) - vec.at( offsetX0Y0ZM ) )  ;
            }
        }
    }

    // Compute derivatives for boundaries: 6 faces of box.
    // In some situations, these macros compute extraneous data.
    // A tiny bit more efficiency could be squeezed from this routine,
    // but it turns out to be well under 1% of the total expense.

#define COMPUTE_FINITE_DIFF                                                                                                             \
    float3x3 & rMatrix = jacobian.data()[ offsetX0Y0Z0 ] ;                                                                                        \
    if( index[0] == 0 )                     { rMatrix.r[0] =reciprocalSpacing[0] * ( vec.at( offsetXPY0Z0 ) - vec.at( offsetX0Y0Z0 ) ) ;     }   \
    else if( index[0] == dimsMinus1[0] )    { rMatrix.r[0] =reciprocalSpacing[0] * ( vec.at( offsetX0Y0Z0 ) - vec.at( offsetXMY0Z0 ) ) ;     }   \
    else                                    { rMatrix.r[0] =halfReciprocalSpacing[0]*  ( vec.at( offsetXPY0Z0 ) - vec.at( offsetXMY0Z0 ) )   ; }   \
    if( index[1] == 0 )                     { rMatrix.r[1] =reciprocalSpacing[1] *  ( vec.at( offsetX0YPZ0 ) - vec.at( offsetX0Y0Z0 ) )  ;     }   \
    else if( index[1] == dimsMinus1[1] )    { rMatrix.r[1] =reciprocalSpacing[1] * ( vec.at( offsetX0Y0Z0 ) - vec.at( offsetX0YMZ0 ) ) ;     }   \
    else                                    { rMatrix.r[1] =halfReciprocalSpacing[1] * ( vec.at( offsetX0YPZ0 ) - vec.at( offsetX0YMZ0 ) )  ; }   \
    if( index[2] == 0 )                     { rMatrix.r[2] =reciprocalSpacing[2] *  ( vec.at( offsetX0Y0ZP ) - vec.at( offsetX0Y0Z0 ) )   ;     }   \
    else if( index[2] == dimsMinus1[2] )    { rMatrix.r[2] =reciprocalSpacing[2] * ( vec.at( offsetX0Y0Z0 ) - vec.at( offsetX0Y0ZM ) ) ;     }   \
    else                                    { rMatrix.r[2]= halfReciprocalSpacing[2]* ( vec.at( offsetX0Y0ZP ) - vec.at( offsetX0Y0ZM ) ) ; }

    // Compute derivatives for -X boundary.
    index[0] = 0 ;
    for( index[2] = 0 ; index[2] < dims[2] ; ++ index[2] )
    {
        ASSIGN_Z_OFFSETS ;
        for( index[1] = 0 ; index[1] < dims[1] ; ++ index[1] )
        {
            ASSIGN_YZ_OFFSETS ;
            {
                ASSIGN_XYZ_OFFSETS ;
                COMPUTE_FINITE_DIFF ;
            }
        }
    }

    // Compute derivatives for -Y boundary.
    index[1] = 0 ;
    for( index[2] = 0 ; index[2] < dims[2] ; ++ index[2] )
    {
        ASSIGN_Z_OFFSETS ;
        {
            ASSIGN_YZ_OFFSETS ;
            for( index[0] = 0 ; index[0] < dims[0] ; ++ index[0] )
            {
                ASSIGN_XYZ_OFFSETS ;
                COMPUTE_FINITE_DIFF ;
            }
        }
    }

    // Compute derivatives for -Z boundary.
    index[2] = 0 ;
    {
        ASSIGN_Z_OFFSETS ;
        for( index[1] = 0 ; index[1] < dims[1] ; ++ index[1] )
        {
            ASSIGN_YZ_OFFSETS ;
            for( index[0] = 0 ; index[0] < dims[0] ; ++ index[0] )
            {
                ASSIGN_XYZ_OFFSETS ;
                COMPUTE_FINITE_DIFF ;
            }
        }
    }

    // Compute derivatives for +X boundary.
    index[0] = dimsMinus1[0] ;
    for( index[2] = 0 ; index[2] < dims[2] ; ++ index[2] )
    {
        ASSIGN_Z_OFFSETS ;
        for( index[1] = 0 ; index[1] < dims[1] ; ++ index[1] )
        {
            ASSIGN_YZ_OFFSETS ;
            {
                ASSIGN_XYZ_OFFSETS ;
                COMPUTE_FINITE_DIFF ;
            }
        }
    }


    // Compute derivatives for +Y boundary.
    index[1] = dimsMinus1[1] ;
    for( index[2] = 0 ; index[2] < dims[2] ; ++ index[2] )
    {
        ASSIGN_Z_OFFSETS ;
        {
            ASSIGN_YZ_OFFSETS ;
            for( index[0] = 0 ; index[0] < dims[0] ; ++ index[0] )
            {
                ASSIGN_XYZ_OFFSETS ;
                COMPUTE_FINITE_DIFF ;
            }
        }
    }

    // Compute derivatives for +Z boundary.
    index[2] = dimsMinus1[2] ;
    {
        ASSIGN_Z_OFFSETS ;
        for( index[1] = 0 ; index[1] < dims[1] ; ++ index[1] )
        {
            ASSIGN_YZ_OFFSETS ;
            for( index[0] = 0 ; index[0] < dims[0] ; ++ index[0] )
            {
                ASSIGN_XYZ_OFFSETS ;
                COMPUTE_FINITE_DIFF ;
            }
        }
    }

#undef COMPUTE_FINITE_DIFF
#undef ASSIGN_XYZ_OFFSETS
#undef ASSIGN_YZ_OFFSETS
#undef ASSIGN_Z_OFFSETS

}

}  // namespace procedural::fluid
