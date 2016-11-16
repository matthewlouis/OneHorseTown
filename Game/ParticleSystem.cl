
struct Particle
{
    float    lifetime;
    float4   color;
    float2   position;
    float2   velocity;
};

__kernel void update_system( __global struct Particle* particles, float time )
{
    const float gravity = -100.0;
    int i = get_global_id( 0 );

    particles[ i ].lifetime -= time;
    particles[ i ].color.w -= time / 3;
    particles[ i ].velocity.y += gravity * time;
    particles[ i ].position += particles[ i ].velocity * time;
}