#include <swap.hpp>
/*
This is the EFi V2 Swap Contract.
*/
void swapcontrak::setbonus(uint8_t bonus)
{
    require_auth( get_self() );
    totaltable totalswapped(get_self(), "totals"_n.value); 

    auto it = totalswapped.find("totals"_n.value);
    if( it != totalswapped.end() ) 
    {
        totalswapped.modify(it, get_self(), [&]( auto& row ) 
        {
            row.bonus = bonus;
        });
    } 
    else 
        return;
}
/* Setter function that needs to be called after the contract is deployed to initialize the totals table (with 0, preferably, except the bonus, maybe.) */
void swapcontrak::set(const asset& initial_hub, const asset& initial_dop, const asset& initial_dmd, uint8_t initial_bonus, bool initial_locked)
{
    require_auth( get_self() );
    totaltable totalswapped(get_self(), "totals"_n.value); 

    auto it = totalswapped.find("totals"_n.value);
    if( it == totalswapped.end() ) 
    {
        totalswapped.emplace( get_self(), [&]( auto& row ) 
        {
            row.key = "totals"_n;
            row.hub_total_swapped = initial_hub;
            row.dmd_total_swapped = initial_dmd;
            row.dop_total_swapped = initial_dop;
            row.bonus             = initial_bonus;
            row.locked            = initial_locked;
        });
    } 
    else 
        return;
}
/* Three very similar eosio::on_notify functions that update the tables whenever users send DMD/DOP/HUB to the contract and then gives back the rewards + bonus.
   The EFi team freeze check is here. If they send tokens, we freeze the whole swap, but keep receiving user tokens. With an added twist. */
[[eosio::on_notify("eoshubtokens::transfer")]]
void swapcontrak::registerswaphub(const name& owner_account, const name& to, const asset& swap_quantity_hub, std::string memo)
{
    if (to != get_self() || owner_account == get_self())
    {
        print("error: these are not the droids you are looking for.");
        return;
    }

    check(swap_quantity_hub.amount > 0, "error: when pigs fly!");
    check(swap_quantity_hub.symbol == hub_symbol, "error: these are not the droids you are looking for.");

    totaltable totalswapped(get_self(), "totals"_n.value); 
    auto total_it = totalswapped.find("totals"_n.value);
    check(total_it != totalswapped.end(), "error: totals table is not initiated.");
    check(total_it->locked == false,"error: the EFi V2 swap is currently locked (inactive).");

    uint8_t bonus = total_it->bonus;
    totalswapped.modify(total_it, get_self(), [&]( auto& row ) 
    {
        row.key = "totals"_n;
        row.hub_total_swapped += swap_quantity_hub; 
    });

    swaptable swapped( get_self(), get_self().value );

    auto swapped_it = swapped.find( owner_account.value );
    if( swapped_it == swapped.end() ) 
    {
        swapped.emplace( get_self(), [&]( auto& row )
        {
            row.owner_account = owner_account;
            row.hub_swapped_amount = swap_quantity_hub; 
        });
    } 
    else 
    {
        swapped.modify(swapped_it, get_self(), [&]( auto& row ) 
        {   
            row.hub_swapped_amount += swap_quantity_hub;
        });
    }

    /* Calculate the rewards by looking at the bonus, if bonus == 0 then just: */
    if (bonus == 0)
    {
        swapcontrak::inline_transferhub(get_self(), owner_account, swap_quantity_hub, "Sending rewards with zero bonus.");
        /* Modify received_amount in the swapped table */
        swapped.modify(swapped_it, get_self(), [&]( auto& row ) 
        {   
            if (row.hub_received_amount.amount == 0)
                row.hub_received_amount = swap_quantity_hub; // Let's see if we can delete this if else here and just add the +=
            else
                row.hub_received_amount += swap_quantity_hub;
        });
    }
    /* Otherwise, we send him more coins */
    else
    {
        /* Send the rewards + the bonuses */
        asset new_swap_quantity = swap_quantity_hub + (swap_quantity_hub * bonus/100); // Give the appropiate bonus.
        swapcontrak::inline_transferhub(get_self(), owner_account, new_swap_quantity, "Sending rewards with bonus.");
        /* Modify received_amount in the swapped table */
        swapped.modify(swapped_it, get_self(), [&]( auto& row ) 
        {   
            if (row.hub_received_amount.amount == 0)
                row.hub_received_amount = new_swap_quantity;
            else
                row.hub_received_amount += new_swap_quantity;
        });
    }
}

[[eosio::on_notify("dolphintoken::transfer")]]
void swapcontrak::registerswapdop(const name& owner_account, const name& to, const asset& swap_quantity_dop, std::string memo)
{
    if (to != get_self() || owner_account == get_self())
    {
        print("error: these are not the droids you are looking for.");
        return;
    }

    check(swap_quantity_dop.amount > 0, "error: when pigs fly!");
    check(swap_quantity_dop.symbol == dop_symbol, "error: these are not the droids you are looking for.");

    totaltable totalswapped(get_self(), "totals"_n.value); 
    auto total_it = totalswapped.find("totals"_n.value);
    check(total_it != totalswapped.end(), "error: totals table is not initiated.");
    check(total_it->locked == false,"error: the EFi V2 swap is currently locked (inactive).");

    uint8_t bonus = total_it->bonus;
    totalswapped.modify(total_it, get_self(), [&]( auto& row ) 
    {
        row.key = "totals"_n;
        row.dop_total_swapped += swap_quantity_dop; 
    });

    swaptable swapped( get_self(), get_self().value );

    auto swapped_it = swapped.find( owner_account.value );
    if( swapped_it == swapped.end() ) 
    {
        swapped.emplace( get_self(), [&]( auto& row )
        {
            row.owner_account = owner_account;
            row.dop_swapped_amount = swap_quantity_dop; 
        });
    } 
    else 
    {
        swapped.modify(swapped_it, get_self(), [&]( auto& row ) 
        {   
            row.dop_swapped_amount += swap_quantity_dop;
        });
    }

    /* Calculate the rewards by looking at the bonus, if bonus == 0 then just: */
    if (bonus == 0)
    {
        swapcontrak::inline_transferdop(get_self(), owner_account, swap_quantity_dop, "Sending rewards with zero bonus.");
        /* Modify received_amount in the swapped table */
        swapped.modify(swapped_it, get_self(), [&]( auto& row ) 
        {   
            if (row.dop_received_amount.amount == 0)
                row.dop_received_amount = swap_quantity_dop; // Let's see if we can delete this if else here and just add the +=
            else
                row.dop_received_amount += swap_quantity_dop;
        });
    }
    /* Otherwise, we send him more coins */
    else
    {
        /* Send the rewards + the bonuses */
        asset new_swap_quantity = swap_quantity_dop + (swap_quantity_dop * bonus/100); // Give the appropiate bonus.
        swapcontrak::inline_transferdop(get_self(), owner_account, new_swap_quantity, "Sending rewards with bonus.");
        /* Modify received_amount in the swapped table */
        swapped.modify(swapped_it, get_self(), [&]( auto& row ) 
        {   
            if (row.dop_received_amount.amount == 0)
                row.dop_received_amount = new_swap_quantity;
            else
                row.dop_received_amount += new_swap_quantity;
        });
    }
}

[[eosio::on_notify("eosdmdtokens::transfer")]]
void swapcontrak::registerswapdmd(const name& owner_account, const name& to, const asset& swap_quantity_dmd, std::string memo)
{
    if (to != get_self() || owner_account == get_self())
    {
        print("error: these are not the droids you are looking for.");
        return;
    }

    check(swap_quantity_dmd.amount > 0, "error: when pigs fly!");
    check(swap_quantity_dmd.symbol == dmd_symbol, "error: these are not the droids you are looking for.");

    totaltable totalswapped(get_self(), "totals"_n.value); 
    auto total_it = totalswapped.find("totals"_n.value);
    check(total_it != totalswapped.end(), "error: totals table is not initiated.");
    check(total_it->locked == false,"error: the EFi V2 swap is currently locked (inactive).");

    uint8_t bonus = total_it->bonus;
    totalswapped.modify(total_it, get_self(), [&]( auto& row ) 
    {
        row.key = "totals"_n;
        row.dmd_total_swapped += swap_quantity_dmd; 
    });

    swaptable swapped( get_self(), get_self().value );

    auto swapped_it = swapped.find( owner_account.value );
    if( swapped_it == swapped.end() ) 
    {
        swapped.emplace( get_self(), [&]( auto& row )
        {
            row.owner_account = owner_account;
            row.dmd_swapped_amount = swap_quantity_dmd; 
        });
    } 
    else 
    {
        swapped.modify(swapped_it, get_self(), [&]( auto& row ) 
        {   
            row.dmd_swapped_amount += swap_quantity_dmd;
        });
    }

    /* Calculate the rewards by looking at the bonus, if bonus == 0 then just: */
    if (bonus == 0)
    {
        swapcontrak::inline_transferdmd(get_self(), owner_account, swap_quantity_dmd, "Sending rewards with zero bonus.");
        /* Modify received_amount in the swapped table */
        swapped.modify(swapped_it, get_self(), [&]( auto& row ) 
        {   
            if (row.dmd_received_amount.amount == 0)
                row.dmd_received_amount = swap_quantity_dmd; // Let's see if we can delete this if else here and just add the +=
            else
                row.dmd_received_amount += swap_quantity_dmd;
        });
    }
    /* Otherwise, we send him more coins */
    else
    {
        /* Send the rewards + the bonuses */
        asset new_swap_quantity = swap_quantity_dmd + (swap_quantity_dmd * bonus/100); // Give the appropiate bonus.
        swapcontrak::inline_transferdmd(get_self(), owner_account, new_swap_quantity, "Sending rewards with bonus.");
        /* Modify received_amount in the swapped table */
        swapped.modify(swapped_it, get_self(), [&]( auto& row ) 
        {   
            if (row.dmd_received_amount.amount == 0)
                row.dmd_received_amount = new_swap_quantity;
            else
                row.dmd_received_amount += new_swap_quantity;
        });
    }
}
