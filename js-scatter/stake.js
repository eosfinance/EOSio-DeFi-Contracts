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
            fname_dmdstakeamount.value = "0.00";
        else
        {
            fname_dmdstakeamount.value = (Math.round(parseFloat(data[0].split(' ')[0]) *1000)/1000).toString();
        }
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
            fname_hubstakeamount.value = "0.0";
        else
        {
            fname_hubstakeamount.value = (Math.round(parseFloat(data[0].split(' ')[0]) *100)/100).toString();
        }
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
            fname_dopstakeamount.value = "0.0";
        else
        {
            fname_dopstakeamount.value = (Math.round(parseFloat(data[0].split(' ')[0]) *100)/100).toString();
        }
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
		v2_staking_title.hidden = true;
		stake_dmd_logo.hidden = true;
		stake_hub_logo.hidden = true;
		stake_dop_logo.hidden = true;
		stake_dmd_inputform.hidden = true;
		stake_hub_inputform.hidden = true;
		stake_dop_inputform.hidden = true;
		info_table.hidden = true;
		usefulinfo.hidden = true;
		swap_invitation.hidden = true;
		unclaimed_dmd_readonly.hidden = true;
		unclaimed_hub_readonly.hidden = true;
		unclaimed_dop_readonly.hidden = true;
		dmd_apr.hidden = true;
		hub_apr.hidden = true;
		dop_apr.hidden = true;

		username.innerText = "No Scatter Detected";
		return;
	}
	if (!ScatterJS.identity) 
	{
		v2_staking_title.hidden = true;
		stake_dmd_logo.hidden = true;
		stake_hub_logo.hidden = true;
		stake_dop_logo.hidden = true;
		stake_dmd_inputform.hidden = true;
		stake_hub_inputform.hidden = true;
		stake_dop_inputform.hidden = true;
		info_table.hidden = true;
		usefulinfo.hidden = true;
		swap_invitation.hidden = true;
		unclaimed_dmd_readonly.hidden = true;
		unclaimed_hub_readonly.hidden = true;
		unclaimed_dop_readonly.hidden = true;
		dmd_apr.hidden = true;
		hub_apr.hidden = true;
		dop_apr.hidden = true;

		username.innerText = "No identity Detected";
		return;
	}

	v2_staking_title.hidden = false;
	stake_dmd_logo.hidden = false;
	stake_hub_logo.hidden = false;
	stake_dop_logo.hidden = false;
	stake_dmd_inputform.hidden = false;
	stake_hub_inputform.hidden = false;
	stake_dop_inputform.hidden = false;
	info_table.hidden = false;
	usefulinfo.hidden = false;
	swap_invitation.hidden = false;
	unclaimed_dmd_readonly.hidden = false;
	unclaimed_hub_readonly.hidden = false;
	unclaimed_dop_readonly.hidden = false;
	dmd_apr.hidden = false;
	hub_apr.hidden = false;
	dop_apr.hidden = false;

	username.innerText = "EOS Account: "+ScatterJS.identity.accounts[0].name;
	usefulinfo.innerHTML = "<strong style=\"color:black\">Useful Information ("+ScatterJS.identity.accounts[0].name+")</strong>";

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

setStatus();
setInterval(() =>
{
	setStatus();
}, 4000);
