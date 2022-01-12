#include <burn.hpp>
/*
A simple contract, to which users send DOP, HUB or DMD, and then the contract burns the coins.
*/

[[eosio::on_notify("dop.efi::transfer")]]
void burncontrak::burndop(const name& owner_account, const name& to, const asset& burn_quantity_dop, std::string memo)
{
    if (to != get_self() || owner_account == get_self())
    {
        print("*these are not the droids you are looking for*");
        return;
    }

    check(owner_account == "funds.efi"_n, "error: the burning is not yet open to the public.");
    check(burn_quantity_dop.amount >= 100000, "error: must burn a minimum of 10 DOP");
    check(burn_quantity_dop.symbol == dop_symbol, "error: these are not the droids you are looking for.");

    std::string custom_memo;

    if (burn_quantity_dop.amount <= 1000000)
        custom_memo = "A pitiful sacrifice!";
    else if ((burn_quantity_dop.amount >= 1000000) && (burn_quantity_dop.amount < 10000000))
        custom_memo = "A worthy sacrifice to the Market Gods!";
    else if ((burn_quantity_dop.amount >= 10000000) && (burn_quantity_dop.amount < 100000000))
        custom_memo = "An incredible sacrifice to the Market Gods!";
    else if ((burn_quantity_dop.amount >= 100000000) && (burn_quantity_dop.amount < 1000000000))
        custom_memo = "A godlike sacrifice to the Market Gods.";
    else
        custom_memo = "Wesa goen tada moon, sers!";


    burncontrak::inline_burndop(get_self(), burn_quantity_dop, custom_memo);
}

[[eosio::on_notify("hub.efi::transfer")]]
void burncontrak::burnhub(const name& owner_account, const name& to, const asset& burn_quantity_hub, std::string memo)
{
    if (to != get_self() || owner_account == get_self())
    {
        print("*these are not the droids you are looking for*");
        return;
    }

    check(owner_account == "funds.efi"_n, "error: the burning is not yet open to the public.");
    check(burn_quantity_hub.amount >= 100000, "error: must burn a minimum of 10 HUB");
    check(burn_quantity_hub.symbol == hub_symbol, "error: these are not the droids you are looking for.");

    std::string custom_memo;

    if (burn_quantity_hub.amount <= 1000000)
        custom_memo = "A pitiful sacrifice!";
    else if ((burn_quantity_hub.amount >= 1000000) && (burn_quantity_hub.amount < 10000000))
        custom_memo = "A worthy sacrifice to the Market Gods!";
    else if ((burn_quantity_hub.amount >= 10000000) && (burn_quantity_hub.amount < 100000000))
        custom_memo = "An incredible sacrifice to the Market Gods!";
    else if ((burn_quantity_hub.amount >= 100000000) && (burn_quantity_hub.amount < 1000000000))
        custom_memo = "A godlike sacrifice to the Market Gods.";
    else
        custom_memo = "Wesa goen tada moon, sers!";


    burncontrak::inline_burnhub(get_self(), burn_quantity_hub, custom_memo);
}

[[eosio::on_notify("dmd.efi::transfer")]]
void burncontrak::burndmd(const name& owner_account, const name& to, const asset& burn_quantity_dmd, std::string memo)
{
    if (to != get_self() || owner_account == get_self())
    {
        print("*these are not the droids you are looking for*");
        return;
    }

    check(owner_account == "funds.efi"_n, "error: the burning is not yet open to the public.");
    check(burn_quantity_dmd.amount >= 100000, "error: must burn a minimum of 10 DMD");
    check(burn_quantity_dmd.symbol == dmd_symbol, "error: these are not the droids you are looking for.");

    std::string custom_memo;

    if (burn_quantity_dmd.amount <= 1000000)
        custom_memo = "A pitiful sacrifice!";
    else if ((burn_quantity_dmd.amount >= 1000000) && (burn_quantity_dmd.amount < 10000000))
        custom_memo = "A worthy sacrifice to the Market Gods!";
    else if ((burn_quantity_dmd.amount >= 10000000) && (burn_quantity_dmd.amount < 100000000))
        custom_memo = "An incredible sacrifice to the Market Gods!";
    else if ((burn_quantity_dmd.amount >= 100000000) && (burn_quantity_dmd.amount < 1000000000))
        custom_memo = "A godlike sacrifice to the Market Gods.";
    else
        custom_memo = "Wesa goen tada moon, sers!";


    burncontrak::inline_burndmd(get_self(), burn_quantity_dmd, custom_memo);
}

void burncontrak::set(uint32_t player_limit)
{
    require_auth(get_self());

    totaltable burnerdata(get_self(), "totals"_n.value); 
    auto total_it = burnerdata.find("totals"_n.value);
    if(total_it != burnerdata.end()) 
    {
        burnerdata.modify(total_it, get_self(), [&]( auto& row) 
        {
            row.player_limit = player_limit;
        });
    } 
    else 
        return;
}
