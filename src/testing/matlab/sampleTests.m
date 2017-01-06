
close; % close previous figures
clc;   % clear console
clear; % clear all variables

numSamples = 1000;
samples    = zeros( numSamples, 3 );
normal     = [ 1, 1, 1 ];
normal     = normal / norm( normal );
point      = [ 1.5, 0.0, 0.0 ];

for i=1:numSamples
   
%     samples( i, : ) = sampleSphere( );
%     samples( i, : ) = sampleHalfSphere( normal );
    samples( i, : ) = sampleSolidAngle( point );
    
end

r = 1.0;
[X,Y,Z] = sphere();
X = X * r;
Y = Y * r;
Z = Z * r;
surf(X,Y,Z);
hold on;

% scatter3( point(:,1), point(:,2), point(:,3), 'k');
% 
% scatter3( samples( :, 1 ), samples( :, 2 ), samples( :, 3 ), 'k' );
% 
% 
% point = [3, 0, 0];
% 
% for i=1:numSamples
%    
%     samples( i, : ) = sampleSolidAngle( point );
%     
% end
% 
% scatter3( point(:,1), point(:,2), point(:,3), 'm');
% scatter3( samples( :, 1 ), samples( :, 2 ), samples( :, 3 ), 'm' );


point = [1.01, 0, 0];

for i=1:numSamples
   
    samples( i, : ) = sampleSolidAngle( point );
    
end

d = norm( point );
cosA = 1.0 - cos(asin(1.0/d));
disp( cosA );
disp( rad2deg( acos( cosA ) ) );

scatter3( point(:,1), point(:,2), point(:,3), 'b');
scatter3( samples( :, 1 ), samples( :, 2 ), samples( :, 3 ), 'b' );
hold off;

