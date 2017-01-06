function [ sample ] = sampleHalfSphere( normal )
%SAMPLEHALFSPHERE Summary of this function goes here
%   Detailed explanation goes here

    sample = sampleSphere( );
    
    if ( dot( sample, normal ) < 0.0 )
       sample = -sample;
    end

end

