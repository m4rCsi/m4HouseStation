Array.prototype.insert = function (index, item)
{
    this.splice(index, 0, item);
};

function CSVToArray( strData, strDelimiter )
{
    // Check to see if the delimiter is defined. If not,
    // then default to comma.
    strDelimiter = (strDelimiter || ",");

    // Create a regular expression to parse the CSV values.
    var objPattern = new RegExp(
        (
            // Delimiters.
            "(\\" + strDelimiter + "|\\r?\\n|\\r|^)" +

                // Quoted fields.
                "(?:\"([^\"]*(?:\"\"[^\"]*)*)\"|" +

                // Standard fields.
                "([^\"\\" + strDelimiter + "\\r\\n]*))"
            ),
        "gi"
    );


    // Create an array to hold our data. Give the array
    // a default empty first row.
    var arrData = [[]];

    // Create an array to hold our individual pattern
    // matching groups.
    var arrMatches = null;


    // Keep looping over the regular expression matches
    // until we can no longer find a match.
    while (arrMatches = objPattern.exec( strData )){

        // Get the delimiter that was found.
        var strMatchedDelimiter = arrMatches[ 1 ];

        // Check to see if the given delimiter has a length
        // (is not the start of string) and if it matches
        // field delimiter. If id does not, then we know
        // that this delimiter is a row delimiter.
        if (
            strMatchedDelimiter.length &&
                (strMatchedDelimiter != strDelimiter)
            ){

            // Since we have reached a new row of data,
            // add an empty row to our data array.
            arrData.push( [] );

        }


        // Now that we have our delimiter out of the way,
        // let's check to see which kind of value we
        // captured (quoted or unquoted).
        if (arrMatches[ 2 ]){

            // We found a quoted value. When we capture
            // this value, unescape any double quotes.
            var strMatchedValue = arrMatches[ 2 ].replace(
                new RegExp( "\"\"", "g" ),
                "\""
            );

        } else {

            // We found a non-quoted value.
            var strMatchedValue = arrMatches[ 3 ];

        }


        // Now that we have our value string, let's add
        // it to the data array.
        arrData[ arrData.length - 1 ].push( strMatchedValue );
    }

    // Return the parsed data.
    return( arrData );
}

function UnixTStoTime(unix_timestamp)
{
// create a new javascript Date object based on the timestamp
// multiplied by 1000 so that the argument is in milliseconds, not seconds
    var date = new Date(unix_timestamp*1000);
// hours part from the timestamp
    var hours = date.getHours();
// minutes part from the timestamp
    var minutes = n(date.getMinutes());
// seconds part from the timestamp
    var seconds = n(date.getSeconds());

    return hours + ':' + minutes + ':' + seconds;
}

function avg(array, from, to)
{
    var total = 0;
    for (var i = from; i <= to; i++)
    {
        total += array[i][1];
    }
    return total/(to-from+1);
}

function average(data, from, to)
{
    for (var i = 0, ii = data.length; i < ii; i++)
    {
        var f = i+from;
        var t = i+to;

        if (f < 0)
        {
            f = 0;
        }
        if (t >= ii)
        {
            t = ii-1;
        }
        data[i][1] = avg(data,f,t);
    }
    return data;
}

function extractData(raw, type)
{
    var data = [];
    // data[0] labels
    // data[1] data
    // data[2] timedif

    for (var i = 0, ii = raw.length; i < ii; i++)
    {
        if (raw[i][0] == type)
        {
            var var1 = Number(raw[i][2]);
            var var2 = Number(raw[i][1]);
            if (isFinite(var1) && isFinite(var2))
            {
                data.push([var1, var2]);
            }
        }
    }
    return data;
}

function extractTimeDifData(raw, type, convert)
{
    var data = [];
    // data[0] labels
    // data[1] data
    // data[2] timedif
    // data[3] raw

    var last = 0;

    for (var i = 0, ii = raw.length; i < ii; i++)
    {
        if (raw[i][0] == type)
        {
            if (last == 0)
            {
                last = Number(raw[i][2]);
            }
            else
            {
                var timedif = Number(raw[i][2]) - last;
                last = Number(raw[i][2]);

                data.push([ last,
                            convert(Number(raw[i][1]),timedif),
                            timedif,
                            Number(raw[i][1])
                          ]);
            }
        }
    }
    return data;
}

function extractMeterData(raw, type, coeff)
{
    var data = [];
    var last = -1;

    for (var i = 0, ii = raw.length; i < ii; i++)
    {
        if (raw[i][0] == type)
        {
            if (last == -1)
            {
                last = i;
                data.push([Number(raw[i][2]), 0, Number(raw[i][1])]);
            }
            else
            {
                var timedif = Number(raw[i][2])-Number(raw[last][2]);
                var val = Number(raw[i][1])-Number(raw[last][1]);
                if (timedif < 60*60*24-240)
                {
                    // discard this
                    last = i;
                    continue;

                }
                else if ( timedif < 60*60*24+240)
                {
                    // assume as 1 day
                    val = val+1;
                }
                else
                {
                    // adjust
                    val = (60*60*24)*val/timedif
                }
                data.push([ Number(raw[i][2]),
                            val*coeff,
                            Number(raw[i][1])]);
                last = i;
            }
        }
    }
    return data;
}

function convertToElePower(value, timedif)
{
    return (4500*value)/timedif;
}

//0.01*60*60
function convertToGasPower(value, timedif)
{
    return (36*value)/timedif;
}

function insertSet(data, i, val0, val1, val2, val3)
{
    data.insert(i, [val0, val1, val2, val3]);
}

function processGas(data,avgtimedif)
{
    var avg_points = [];
    var avg = [];

    for (var i = 1; i < data.length; i++)
    {
        if (data[i][2] >= 25) // split it
        {
            var time1 = Number(data[i-1][0]);
            var time2 = Number(data[i][0]);
            var dif = time2 - time1;
            var oldValue = Number(data[i][3]);

            insertSet(data,i++,time1+9, convertToGasPower(oldValue/2,9),    9,      oldValue/2);
            insertSet(data,i++,time1+9, 0,                                  0,      0);
            insertSet(data,i++,time2-9, 0,                                  dif-18, 0);
            insertSet(data,i++,time2-9, convertToGasPower(oldValue/2,9),    0,      oldValue/2);
            data[i][1] =                convertToGasPower(oldValue/2,9);
            data[i][2] =                                                    9;
            data[i][3] =                                                            oldValue/2;

            if (dif-18 < avgtimedif)
            {
                avg_points.push([i-2,true]);
            }
            else
            {
                avg_points.push([i-3,false]);
                avg_points.push([i-2,false]);
            }
        }
    }

    for (var i = 1, ii = avg_points.length; i < ii; i++)
    {
        var from = avg_points[i-1][0];
        var to = avg_points[i-0][0];
        var sum = 0;
        var Tfrom = data[from][0];
        var Tto = data[to][0];
        if (avg_points[i-1][1])
        {
            Tfrom -= data[from][2]/2
        }
        if (avg_points[i-0][1])
        {
            Tto += data[to][2]/2;
        }

        var timedif = Tto - Tfrom;

        for (var j = from; j < to; j++)
        {
            sum += data[j][1]*data[j][2];
        }

        avg.push([Tfrom, sum/timedif]);
    }
    return avg;
}

function add(label,data,colors,ladd,dadd, color)
{
    if (ladd.length >= 2)
    {
        var i = label.length;
        label[i] = ladd;
        data[i] = dadd;
        colors[i] = color;
    }
}

function n(n)
{
    return n > 9 ? "" + n: "0" + n;
}

function toDay(data)
{
    for(var i = 0, ii = data.length; i < ii; i++)
    {
        var date = new Date(data[i][0]*1000)
        data[i][0] = n(date.getDate())+"/"+n(date.getMonth()+1)+"/"+date.getFullYear();
    }
}

function toDate(data)
{
    for(var i = 0, ii = data.length; i < ii; i++)
    {
        data[i][0] = data[i][0]*1000;
    }
}

function env_diagram(stringData,renderTo)
{
    setEnvStatus("",1);
    var raw =CSVToArray(stringData);

    var temp = extractData(raw, "Temp");
    var hum = extractData(raw, "Hum");

    toDate(temp);
    toDate(hum);

    var title = "Temperature and Humidity";
    var subtitle = "45 Westway on " + n(env_date.getDate()) + "/" + n(env_date.getMonth()+1) + "/" + env_date.getFullYear();

    var chart = new Highcharts.Chart(
        {   chart: {
                renderTo: renderTo,
                alignTicks: false
                    },
            title: {text: title},
            subtitle: {text: subtitle},
            xAxis: {
                type: 'datetime'
            },
            yAxis: [{ // Primary yAxis
                labels: {
                    formatter: function() {
                        return this.value +'°C';
                    },
                    style: {
                        color: '#AA4643'
                    }
                },
                title: {
                    text: 'Temperature',
                    style: {
                        color: '#AA4643'
                    }
                },
                opposite: false,
                min: 0
            }, { // Secondary yAxis
                gridLineWidth: 0,
                title: {
                    text: 'Humidity',
                    style: {
                        color: '#4572A7'
                    }
                },
                labels: {
                    formatter: function() {
                        return this.value +' % RH';
                    },
                    style: {
                        color: '#4572A7'
                    }
                },
                opposite: true,
                min: 0,
                max: 100,
                gridLineWidth: 0
            }],
            series: [
            {
                yAxis: 0,
                data: temp,
                name: 'Temperature',
                color: '#AA4643'
            },
            {
                yAxis: 1,
                data: hum,
                name: 'Humidity',
                color: '#4572A7'
            }]
        });
}

function countD(data)
{
    var count = 0;
    for(var i = 0, ii = data.length; i < ii; i++)
    {
        count += data[i][3];
    }
    return count;
}

function displayPowStats(eleImp,gasImp)
{
    var eleKwh = eleImp * 0.00125;
    var gasm3 = gasImp * 0.01;

    var elePound = 0.1355 * eleKwh;
    var gasKwh = gasm3 * 11.049;
    var gasPound = 0.04042 * gasKwh;

    document.getElementById("ps_ele_imp").innerHTML=eleImp + " [1/800 Imp/kWh]";
    document.getElementById("ps_ele_kwh").innerHTML=eleKwh.toFixed(3) + " kWh";
    document.getElementById("ps_ele_price").innerHTML= "£" +elePound.toFixed(2);
    document.getElementById("ps_gas_imp").innerHTML=gasImp + " [0.01 Imp/m3]";
    document.getElementById("ps_gas_m3").innerHTML=gasm3.toFixed(3) + " m3";
    document.getElementById("ps_gas_kwh").innerHTML=gasKwh.toFixed(3) + " kWh";
    document.getElementById("ps_gas_price").innerHTML="£" + gasPound.toFixed(2);

    $('#pow_stats').show();
}

function pow_diagramOverview(stringData,renderTo)
{
    setPowStatus("",1);
    $('#pow_stats').hide();

    var raw =CSVToArray(stringData);

    var ele = extractMeterData(raw, "elemeter", 1/10);
    var gas = extractMeterData(raw, "gasmeter", 1/100);

    toDate(ele);
    toDate(gas);

    var title = "Power Consumption";
    var subtitle = "45 Westway overview";

    var chart = new Highcharts.Chart(
        {   chart: {
            renderTo: renderTo,
            zoomType: 'x'
        },
            title: {text: title},
            subtitle: {text: subtitle},
            plotOptions: {
                series: {
                    marker: {
                        enabled: false
                    }
                }
            },

            xAxis: {
                type: 'datetime'
            },
            yAxis: [{ // Primary yAxis
                labels: {
                    formatter: function() {
                        return this.value +' kWh';
                    },
                    style: {
                        color: '#AA4643'
                    }
                },
                title: {
                    text: 'Electricity usage per day',
                    style: {
                        color: '#AA4643'
                    }
                },
                min: 0,
                //max: 2000,
                startOnTick: false,
                opposite: false

            }, { // Secondary yAxis
                gridLineWidth: 0,
                title: {
                    text: 'Gas usage per day',
                    style: {
                        color: '#4572A7'
                    }
                },
                labels: {
                    formatter: function() {
                        return this.value +' m3';
                    },
                    style: {
                        color: '#4572A7'
                    }
                },
                min: 0,
                //max: 5,
                startOnTick: false,
                opposite: true
            },
            { // Primary yAxis Price
                labels: {
                    formatter: function() {
                        return '£' + (this.value * 0.1355).toFixed(2);
                    },
                    style: {
                        color: '#AA4643'
                    }
                },
                title: {
                    text: 'Cost per day',
                    style: {
                        color: '#AA4643'
                    }
                },
                linkedTo:0,
                opposite: false

            },
                { // Secondary yAxis
                    gridLineWidth: 0,
                    title: {
                        text: 'Cost per day',
                        style: {
                            color: '#4572A7'
                        }
                    },
                    labels: {
                        formatter: function() {
                            return "£" + (this.value*11.049*0.04042).toFixed(2);
                        },
                        style: {
                            color: '#4572A7'
                        }
                    },
                    linkedTo:1,
                    opposite: true
                }],
            series: [{
                yAxis: 0,
                data: ele,
                step: "right",
                name: 'Electricity',
                color: '#AA4643'
            },
            {
                yAxis: 1,
                data: gas,
                step: "right",
                name: 'Gas',
                color: '#4572A7'}]

        });
}

function combine(raw)
{
    for(var i = 0; i < raw.length; i++)
    {
        var ts = raw[i][2];
        for (var j = i+1; j < raw.length; j++)
        {
            if (ts != raw[j][2])
            {
                break;
            }
            else
            {
                if (raw[i][0] == raw[j][0])
                {
                    // combine
                    raw[i][1] = Number(raw[i][1]) + Number(raw[j][1]);
                    raw[j][0] = "combined";
                }
            }
        }
    }
}

function pow_diagram(stringData,renderTo)
{
    setPowStatus("",1);
    var raw =CSVToArray(stringData);

    combine(raw);

    var ele = extractTimeDifData(raw, "Ele", convertToElePower);
    var gas = extractTimeDifData(raw, "Gas", convertToGasPower);

    var eleImp = countD(ele);
    var gasImp = countD(gas);

    displayPowStats(eleImp,gasImp);

    var gasavg = processGas(gas,600);

    average(ele,-5,5);

    toDate(ele);
    toDate(gasavg);

    var title = "Power Consumption";
    var subtitle = "45 Westway on " + n(pow_date.getDate()) + "/" + n(pow_date.getMonth()+1) + "/" + pow_date.getFullYear();

    var chart = new Highcharts.Chart(
        {   chart: {
                renderTo: renderTo,
                zoomType: 'x'
                    },
            title: {text: title},
            subtitle: {text: subtitle},
            plotOptions: {
                series: {
                    marker: {
                        enabled: false
                    }
                }
            },

            xAxis: {
                type: 'datetime'
            },
            yAxis: [{ // Primary yAxis
                labels: {
                    formatter: function() {
                        return this.value +'W';
                    },
                    style: {
                        color: '#AA4643'
                    }
                },
                title: {
                    text: 'Electricity usage',
                    style: {
                        color: '#AA4643'
                    }
                },
                min: 0,
                //max: 2000,
                startOnTick: false,
                opposite: false

            }, { // Secondary yAxis
                gridLineWidth: 0,
                title: {
                    text: 'Gas usage',
                    style: {
                        color: '#4572A7'
                    }
                },
                labels: {
                    formatter: function() {
                        return this.value +' m3/h';
                    },
                    style: {
                        color: '#4572A7'
                    }
                },
                min: 0,
                //max: 5,
                startOnTick: false,
                opposite: true
            }],
            series: [{
                    yAxis: 0,
                    data: ele,
                    name: 'Electricity',
                    color: '#AA4643'
                },
                /*{
                    yAxis: 1,
                    data: gas,
                    name: 'Gas',
                    step: false,
                    color: '#4572A7'
                },*/
                {
                    yAxis: 1,
                    data: gasavg,
                    step: true,
                    name: 'Gas',
                    color: '#4572A7'}]

        });
}

function digitShift(leftshift, data)
{
    if (leftshift > 0)
        return data / (Math.pow(10,leftshift));
    return data;
}

var gas_meter = 0;
var ele_meter = 0;

function display_now(stringData, renderTo)
{
    var vars = {};
    vars["gas_meter"] = ["Gas Meter", 2, "m3 <a id=\"adjust_gas_meter\" href=\"#\">adjust</a>"];
    vars["gas_power"] = ["Gas consumption",3, "m3/h"];
    vars["ele_meter"] = ["Ele Meter", 1, "kWh <a id=\"adjust_ele_meter\" href=\"#\">adjust</a>"];
    vars["ele_power"] = ["Ele consumption",0,"W"];
    vars["temp"] =      ["Temperature",0,"°C"];
    vars["hum"] =       ["Humidity",0,"% RH"];

    var raw =CSVToArray(stringData);

    for (var i = 0, ii = raw.length; i < ii; i++)
    {
        var field_name = raw[i][0];
        if (field_name in vars)
        {

            var element=document.getElementById(field_name);
            element.innerHTML=digitShift(vars[field_name][1], raw[i][1])+ " " +vars[field_name][2] ;

            if (field_name == "gas_meter")
            {
                gas_meter = Number(raw[i][1]);
                $(function() {
                    $("#adjust_gas_meter").click(function() {
                        //e.preventDefault(); // if desired...
                        var value = prompt("Adjust Gas Meter",gas_meter);
                        if (value != null)
                        {
                            $.ajax({
                                url:  "/",
                                type: "POST",
                                data: {g: value }
                            })
                                .done( function(data){
                                    alert("Gas Meter adjusted to " + value + ".");
                                });
                            return false;
                        }
                    });
                });
            }
            else if (field_name == "ele_meter")
            {
                ele_meter = Number(raw[i][1]);
                $("#adjust_ele_meter").click(function() {
                    //e.preventDefault(); // if desired...
                    var value = prompt("Adjust Ele Meter",ele_meter);
                    if (value != null)
                    {
                        $.ajax({
                            url:  "/",
                            type: "POST",
                            data: {e: value }
                        })
                            .done( function(data){
                                alert("Ele Meter adjusted to " + value + "." + data);
                            });
                        return false;
                    }
                });
            }
        }
    }
    setNowStatus("",1);
}

var validEDays = [];
var validPDays = [];

function extractDates(filename)
{
    var firstchar = filename.charAt(0);

    if ((firstchar == 'E' ||
        firstchar == 'M')  &&
        filename.length == 11 &&
        filename.substring(7,11) == ".TXT")
    {
        if (firstchar == 'E')
            validEDays.push(Number(filename.substring(1,7)));

        if (firstchar == 'M')
            validPDays.push(Number(filename.substring(1,7)));
    }
}

function applyList(data)
{
    var files = CSVToArray(data);

    for (var i = 0, ii = files.length; i < ii; i++)
    {
        extractDates(files[i][0]);
    }

    validEDays.sort();
    validPDays.sort();
}

function getNow(filename)
{
    $.ajax({
        type: "GET",
        url: filename,
        error: function() {
            setNowStatus("no data",-1);
        },
        success: function(data) {
            display_now(data,"stats");
        }
    });
}

function getEnv(filename)
{
    $.ajax({
        type: "GET",
        url: filename,
        error: function() {
            setEnvStatus("no data for that day",-1);
        },
        success: function(data) {
            env_diagram(data,"env_diagram");
        }
    });
}

function getPow(filename)
{
    $.ajax({
        type: "GET",
        url: filename,
        error: function() {
            setPowStatus("no data for that day",-1);
        },
        success: function(data) {
            pow_diagram(data,"pow_diagram");
        }
    });
}

function getPowOverview(filename)
{
    $.ajax({
        type: "GET",
        url: filename,
        error: function() {
            setPowStatus("no data found",-1);
        },
        success: function(data) {
            pow_diagramOverview(data,"pow_diagram");
        }
    });
}

function checkEDay(date)
{
    var dnum = Number(getDateString(date));
    for (var i = validEDays.length-1; i >= 0; i--)
    {
        if (validEDays[i] < dnum)
            return {0: false}
        if (validEDays[i] == dnum)
            return {0: true}
    }
    return {0: false}
}

function checkPDay(date)
{
    var dnum = Number(getDateString(date));
    for (var i = validEDays.length-1; i >= 0; i--)
    {
        if (validPDays[i] < dnum)
            return {0: false}
        if (validPDays[i] == dnum)
            return {0: true}
    }
    return {0: false}
}

function getList(filename)
{
    $.ajax({
        type: "GET",
        url: filename,
        error: function() {

        },
        success: function(data) {
            applyList(data);
        }
    });
}

function setStatus(status,show,statusID,contentID)
{
    document.getElementById(statusID).innerHTML=status;
    var cont = $(contentID);
    if (show == 1)
    {
        cont.show();
    }
    else if (show == -1)
    {
        cont.hide();
    }
}

function setNowStatus(status,show)
{
    setStatus(status,show,"now_status",'#now_content');
}

function setPowStatus(status,show)
{
    setStatus(status,show,"pow_status",'#pow_content');
}

function setEnvStatus(status,show)
{
    setStatus(status,show,"env_status",'#env_content');
}

env_date = null;
pow_date = null;

function getDateString(date)
{
    return (date.getFullYear()+'').substring(2,4) + n(date.getMonth()+1)+ n(date.getDate());
}

function initDocument()
{
    $('#now_showhide').click(function(){
        $("#now_content").slideToggle();
    });

    $('#pow_showhide').click(function(){
        $("#pow_content").slideToggle();
    });

    $('#env_showhide').click(function(){
        $("#env_content").slideToggle();
    });

    $('#now_load').click(function(){
        getNow("now.txt");
        setNowStatus("loading...");
    });

    $('#pow_load').click(function(){
        pow_date = $( "#pow_datepicker" ).datepicker('getDate');
        if (pow_date == null)
        {
            pow_date = new Date();
        }
        var fn = "M"+getDateString(pow_date);
        getPow(fn + ".txt");
        setPowStatus("loading..." + fn + ".txt");
    });

    $('#pow_load_overview').click(function(){
        getPowOverview("meter.txt");
        setPowStatus("loading..." + "meter.txt");
    });

    $('#env_load').click(function(){
        env_date = $( "#env_datepicker" ).datepicker('getDate');
        if (env_date == null)
        {
            env_date = new Date();
        }
        var fn = "E"+getDateString(env_date);
        getEnv(fn + ".txt");
        setEnvStatus("loading..." + fn + ".txt");
    });

    $(function() {
        $( "#env_datepicker" ).datepicker({
            dateFormat: "dd/mm/yy",
            maxDate: 0,
            beforeShowDay: checkEDay
        });
    });

    $(function() {
        $( "#pow_datepicker" ).datepicker({
            dateFormat: "dd/mm/yy",
            maxDate: 0,
            beforeShowDay: checkPDay
        });
    });
}

$(document).ready(function()
{
    initDocument();

    getList("list.txt");
});