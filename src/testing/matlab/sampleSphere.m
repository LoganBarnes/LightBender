function [ sample ] = sampleSphere( )
%SAMPLESPHERE Summary of this function goes here
%   Detailed explanation goes here

%     sample = [ rand(), rand(), rand() ];
%     sample = sample / norm( sample );
    theta = rand() * 2 * pi;
    u     = rand() * 2.0 - 1.0;
    
    xyCoeff = sqrt( 1.0 - u * u );
    
    sample = [ xyCoeff * cos( theta ),...
               xyCoeff * sin( theta ),...
               u ];
    

end

