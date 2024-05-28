% This script is used to collect BLE devices with MATLAB and record data
% from UART
%% FIND AND CONNECT
clear

list = blelist("Timeout", 1)
finding = true;
while finding
    for i = 1:1:length(list.Name)
        if list.Name(i) == "Stent" % The name of the device
            finding = false;
            break;
        end
    end
    if finding
        disp("nothing")
        list = blelist
    end
end

b = ble("E1CCAE029206"); % The address of the device
c1 = characteristic(b,"6E400001-B5A3-F393-E0A9-E50E24DCCA9E","6E400003-B5A3-F393-E0A9-E50E24DCCA9E")


%% Sample data once

% data = [];
% data = read(c1);
% data_int = uint8(data);
% msg = char(data_int);
% msg

%% Continuously sample data

% init holder 
Bx = zeros(1,10000);
By = Bx;
Bz = Bx;
lig1 = Bx;
cap1 = Bx;
time = Bx;
temperature = Bx;
R0 = 100;

% init figure
fb = figure();
hold on
pBx = plot(Bx);
pBy = plot(By);
pBz = plot(Bz);
hold off
fl = figure();
pLig = plot(lig1);
fc = figure();
pC = plot(cap1);
ft = figure();
pT = plot(temperature);

% offset time
data = read(c1);
data_int = uint8(data);
msg = char(data_int)
values = split(msg,",");
% start_time_milli = str2double(values(6));

i = 1;
while true
    i = i + 1;

    % read and convert to string
    data = read(c1);
    data_int = uint8(data);
    msg = char(data_int);
    
 
    % extract reading values 
    values = split(msg,",");   
    n = length(values);
    if n == 1
        Bx(i) = Bx(i-1);
        By(i) = By(i-1);
        Bz(i) = Bz(i-1);
        cap1(i) = cap1(i-1);
        lig1(i) = lig1(i-1);
        temperature(i) = temperature(i-1); % if msg is NULL, use the previous value
    else
        Bx(i) = str2double(values(1));
        By(i) = str2double(values(2));
        Bz(i) = str2double(values(3));
        cap1(i) = str2double(values(4));
        lig1(i) = str2double(values(6));
        temperature(i) = str2double(values(5));
    end

    if(i > 101)
        figure(fb);
        set(pBx, 'XData', i-100:i, 'YData', Bx(i - 100 : i));
        set(pBy, 'XData', i-100:i, 'YData', By(i - 100 : i));
        set(pBz, 'XData', i-100:i, 'YData', Bz(i - 100 : i));
        axis([i-100 i -50 50])
        legend('Bx','By','Bz');
        ylabel('B (mT)');

        figure(fl);
        set(pLig, 'XData', i-100:i, 'YData', lig1(i - 100 : i));
        axis([i-100 i 0 1000])

        figure(fc);
        set(pC, 'XData', i-100:i, 'YData', cap1(i - 100 : i));
        axis([i-100 i 0 200])
        a = num2str(cap1(i));
        title(a);

        figure(ft);
        set(pT, 'XData', i-100:i, 'YData', temperature(i - 100 : i));
        axis([i-100 i 20 50])
        a = num2str(temperature(i));
        title(a);
    end
end
%% work 
t = datetime("now");
data = [];
data(1,:) = read(c);
x = [];
x(1) = 1;
while true
    data(end + 1,:) = read(c)/10;
    x(end + 1) = x(end) + 1;
    if length(x) > 50
        stairs(data(end - 50 : end, 2));
    else
        stairs(x, data(:,2));
    end
    
    t_now = datetime("now");
%     disp(milliseconds(t_now - t));
    t = t_now;
end