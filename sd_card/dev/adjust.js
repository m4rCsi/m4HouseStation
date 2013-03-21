function display(stringData)
{
    var vars = {};
    vars["gas_meter"] = ["Gas Meter", 2, "m3 <a id=\"adjust_gas_meter\" href=\"#\">adjust</a>"];
    vars["ele_meter"] = ["Ele Meter", 1, "kWh <a id=\"adjust_ele_meter\" href=\"#\">adjust</a>"];

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
                                alert("Ele Meter adjusted to " + value + ".");
                            });
                        return false;
                    }
                });
            }
        }
    }
    setStatus("");
}

function setStatus(status)
{
    document.getElementById("status").innerHTML=status;
}

$(document).ready(function()
{
    $.ajax({
        type: "GET",
        url: "now.txt",
        error: function() {
            alert("nope");
            setStatus("loading failed");
        },
        success: function(data) {
            display(data);
        }
    });
    setStatus("loading");
});