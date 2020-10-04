precision mediump float;


uniform float time;

varying vec4  fragColor;


/*
void main()
{
    gl_FragColor = fragColor;
}
*/
//vec2 u_resolution = 

void main (void) {
    vec2 st = gl_FragCoord.xy/480.; // 0-1
    //vec2 st = vec2(.1);
// vec2 _st = tile(st,7.0); // 7 of 0-1
    
    vec2 grid = abs(st.xy * 7.);
    float idx = grid.x;
    
    // y = 0/1, even/odd
    //float y = step(1.0, mod(_st.x, 2.0));
    vec3 col = vec3(.0, .0, .0);
    //col.rg = _st.xy;
    
    // don't draw first and last coloumn
    if(grid.x <= 1.0 || grid.x >= 6.0){
        col.b = .0;
    } else
    // 2nd and 6th col
    if((grid.x <= 2. || grid.x >= 5.0)
    && (grid.y <= 5. && grid.y >= 1.0))
    {
        col.r = 1.; // paint
    }
    else // 3rd and 5th coloumn
    if((grid.x <= 3. || grid.x >= 4.0)
    &&((grid.y <= 2. && grid.y >= 1.0)
    || (grid.y <= 4. && grid.y >= 3.0) ))
    {
        col.r = 1.;
    }
    else // middle coloumn
    if((grid.x <= 4. && grid.x >= 3.0)
    && (grid.y <= 6. && grid.y >= 3.0))
    {
        col.r = 1.; // paint
    }
    //else // other stuff
    if(idx <= 4.0){
        col.b = 0.;
    } else if(idx <= 5.0){
        col.b = 1.;
    } else if(idx <= 6.0){
        col.b = 0.;
    } else if(idx <= 7.0){
        col.b = 0.;
    }

//    if(y == 0.)    	col.b = 0.5;
//    else        
    col.b = idx /7.;

    gl_FragColor = vec4(col, 1.0);

    gl_FragColor.r *= (sin(time));
        //st = rotateTilePattern(st);

    // Make more interesting combinations
     //st = tile(st,2.0);
     //st = rotate2D(st,-PI*u_time*0.25);
     //st = rotateTilePattern(st*2.);
     //st = rotate2D(st,PI*u_time*0.25);

    // step(st.x,st.y) just makes a b&w triangles
    // but you can use whatever design you want.
    
}