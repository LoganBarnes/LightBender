function [ sample ] = sampleSolidAngle( point )
%SAMPLESOLIDANGLE Summary of this function goes here
%   Detailed explanation goes here

    d = norm( point );
    cosA = cos(asin(1.0/d));
    normal = point / d;
    
    sample = sampleSphere( );
    
    cosSample = dot( sample, normal );
    
    if ( cosSample < 0.0 )
       sample = -sample;
    end
    
    while cosSample <= 1.0 - cosA
        
        sample = sampleSphere( );

        cosSample = dot( sample, normal );

        if ( cosSample < 0.0 )
           sample = -sample;
        end
        
    end

end

