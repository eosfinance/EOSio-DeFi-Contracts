ScatterJS.plugins( new ScatterEOS() );

const network = ScatterJS.Network.fromJson({
    blockchain:'eos',
    protocol:'https',
    host:'eos.greymass.com',
    port:443,
    chainId:'aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906'
});

const rpc = new eosjs_jsonrpc.default(network.fullhost());
const api = ScatterJS.eos(network,eosjs_api.default, rpc);

// Run ScatterJS.Connect
ScatterJS.scatter.connect('SwapDAPP', {network}).then(connected => 
{
    if(!connected) return console.error('no scatter');
        ScatterJS.login().then(id=> 
        {
            if(!id) return console.error('no identity');
        })
});

window.loginScatter = () =>
{
    ScatterJS.scatter.connect('SwapDAPP', {network}).then(connected => 
    {
        if(!connected) return console.error('no scatter');
            ScatterJS.login().then(id=> 
            {
                if(!id) return console.error('no identity');
            })

    })
}

window.logoutScatter = () =>
{
    ScatterJS.scatter.logout();
}

window.maxDMD = () =>
{
    rpc.get_currency_balance('eosdmdtokens', ScatterJS.identity.accounts[0].name, "DMD").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            fname_dmdswapamount.value = "0.00";
        else
        {
            fname_dmdswapamount.value = (Math.round(parseFloat(data[0].split(' ')[0]) *1000)/1000).toString();
        }
        sel = parseInt(fname_dmdswapamount.value)
        display_calculated_bonus_dmd = sel*current_bonus/100;
        bonus_output_calculator_dmd.innerText = display_calculated_bonus_dmd;
        bonus_input_copier_dmd.innerText = fname_dmdswapamount.value+" DMD";
        }).catch(error => 
        {
            console.log(error);
        });
}

window.maxHUB = () =>
{
    rpc.get_currency_balance('eoshubtokens', ScatterJS.identity.accounts[0].name, "HUB").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            fname_hubswapamount.value = "0.0";
        else
        {
            fname_hubswapamount.value = (Math.round(parseFloat(data[0].split(' ')[0]) *100)/100).toString();
        }
        sel = parseInt(fname_hubswapamount.value)
        display_calculated_bonus_hub = sel*current_bonus/100;
        bonus_output_calculator_hub.innerText = display_calculated_bonus_hub;
        bonus_input_copier_hub.innerText = fname_hubswapamount.value+" HUB";
        }).catch(error => 
        {
            console.log(error);
        });
}

window.maxDOP = () =>
{
    rpc.get_currency_balance('dolphintoken', ScatterJS.identity.accounts[0].name, "DOP").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            fname_dopswapamount.value = "0.0";
        else
        {
            fname_dopswapamount.value = (Math.round(parseFloat(data[0].split(' ')[0]) *100)/100).toString();
        }
        sel = parseInt(fname_dopswapamount.value)
        display_calculated_bonus_dop = sel*current_bonus/100;
        bonus_output_calculator_dop.innerText = display_calculated_bonus_dop;
        bonus_input_copier_dop.innerText = fname_dopswapamount.value+" DOP";
        }).catch(error => 
        {
            console.log(error);
        });
}

// Perpetually update the status id
const setStatus = () => 
{
    const status = document.getElementById('status');
    if (!ScatterJS)
    {
        swapCoinsNow.hidden = true;
        swap_dmd_logo.hidden = true;
        swap_hub_logo.hidden = true;
        swap_dop_logo.hidden = true;
        swap_dmd_inputform.hidden = true;
        swap_hub_inputform.hidden = true;
        swap_dop_inputform.hidden = true;
        info_table.hidden = true;
        usefulinfo.hidden = true;
        yourbonus_dmd.hidden = true;
        yourbonus_hub.hidden = true;
        yourbonus_dop.hidden = true;
        todays_bonus_text.hidden = true;
        staking_invitation.hidden = true;

        noUsernameInfo.hidden = false;
        username.innerText = "No Scatter Detected";
        return;
    }
    if (!ScatterJS.identity) 
    {
        swapCoinsNow.hidden = true;
        swap_dmd_logo.hidden = true;
        swap_hub_logo.hidden = true;
        swap_dop_logo.hidden = true;
        swap_dmd_inputform.hidden = true;
        swap_hub_inputform.hidden = true;
        swap_dop_inputform.hidden = true;
        info_table.hidden = true;
        usefulinfo.hidden = true;
        yourbonus_dmd.hidden = true;
        yourbonus_hub.hidden = true;
        yourbonus_dop.hidden = true;
        todays_bonus_text.hidden = true;
        staking_invitation.hidden = true;

        noUsernameInfo.hidden = false;
        username.innerText = "No identity Detected";
        return;
    }

    swapCoinsNow.hidden = false;
    swap_dmd_logo.hidden = false;
    swap_hub_logo.hidden = false;
    swap_dop_logo.hidden = false;
    swap_dmd_inputform.hidden = false;
    swap_hub_inputform.hidden = false;
    swap_dop_inputform.hidden = false;
    info_table.hidden = false;
    usefulinfo.hidden = false;
    yourbonus_dmd.hidden = false;
    yourbonus_hub.hidden = false;
    yourbonus_dop.hidden = false;
    todays_bonus_text.hidden = false;
    staking_invitation.hidden = false;

    noUsernameInfo.hidden = true;
    username.innerText = "EOS Account: "+ScatterJS.identity.accounts[0].name;
    usefulinfo.innerHTML = "<strong style=\"color:black\">Useful Information ("+ScatterJS.identity.accounts[0].name+")</strong>";
    /*
    Show the user his bonus in real-time by checking what is in the swap input field.
    Need to also get the current_bonus variable by looking at the contract table when it's up-and-running. 
    */
    current_bonus = 10;
    current_bonusall.innerText = current_bonus.toString() +"%";

    rpc.get_currency_balance('eosdmdtokens', ScatterJS.identity.accounts[0].name, "DMD").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            dmd_v1_balance.innerText = "0.0000 DMD";
        else
        {
            dmd_v1_balance.innerText = data[0];
            console.log(data[0]);
        }
        }).catch(error => 
        {
            console.log(error);
        });

    rpc.get_currency_balance('eoshubtokens', ScatterJS.identity.accounts[0].name, "HUB").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            hub_v1_balance.innerText = "0.0000 HUB";
        else
        {
            hub_v1_balance.innerText = data[0];
            console.log(data[0]);
        }
        }).catch(error => 
        {
            console.log(error);
        });

    rpc.get_currency_balance('dolphintoken', ScatterJS.identity.accounts[0].name, "DOP").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            dop_v1_balance.innerText = "0.0000 DOP";
        else
        {
            dop_v1_balance.innerText = data[0];
            console.log(data[0]);
        }
        }).catch(error => 
        {
            console.log(error);
        });
}

window.swapInputKeyPress = () =>
{
    console.log(fname_hubswapamount.value);

    if ((document.getElementById("fname_dmdswapamount").value) != "")
        bonus_input_copier_dmd.innerText = document.getElementById("fname_dmdswapamount").value+" DMD";
    else
        bonus_input_copier_dmd.innerText = "0 DMD";

    if ((document.getElementById("fname_hubswapamount").value) != "")
        bonus_input_copier_hub.innerText = document.getElementById("fname_hubswapamount").value+" HUB";
    else
        bonus_input_copier_hub.innerText = "0 HUB";

    if ((document.getElementById("fname_dopswapamount").value) != "")
        bonus_input_copier_dop.innerText = document.getElementById("fname_dopswapamount").value+" DOP";
    else
        bonus_input_copier_dop.innerText = "0 DOP";

    selected_dmd = parseInt(document.getElementById("fname_dmdswapamount").value.split(' ')[0]);
    selected_hub = parseInt(document.getElementById("fname_hubswapamount").value.split(' ')[0]);
    selected_dop = parseInt(document.getElementById("fname_dopswapamount").value.split(' ')[0]);

    if (Number.isInteger(selected_dmd)){
        display_calculated_bonus_dmd = selected_dmd*current_bonus/100;
        bonus_output_calculator_dmd.innerText = display_calculated_bonus_dmd;}
    else
        bonus_output_calculator_dmd.innerText = "0"

    if (Number.isInteger(selected_hub)){
        display_calculated_bonus_hub = selected_hub*current_bonus/100;
        bonus_output_calculator_hub.innerText = display_calculated_bonus_hub;}
    else
        bonus_output_calculator_hub.innerText = "0"

    if (Number.isInteger(selected_dop)){
        display_calculated_bonus_dop = selected_dop*current_bonus/100;
        bonus_output_calculator_dop.innerText = display_calculated_bonus_dop;}
    else
        bonus_output_calculator_dop.innerText = "0"
}
setStatus();
setInterval(() =>
{
    setStatus();
}, 4000);
