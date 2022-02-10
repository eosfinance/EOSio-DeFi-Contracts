/*
Defibox LP non-custodial contract:
*/

void efimine::set(uint32_t hub_issue_frequency, uint32_t dop_issue_frequency, uint32_t dmd_issue_frequency , bool locked)
{
    require_auth(get_self());

    totaltable total_lpstats(get_self(), "totals"_n.value);
    auto total_it = total_lpstats.find("totals"_n.value);

    uint32_t now = current_time_point().sec_since_epoch();
    uint32_t seconds_in_a_month = 2629743;
    uint32_t months_between_halvings = 3;

    if(total_it == total_lpstats.end()) 
    { // Some of these rows will only get modified the first time set() is ran:
        total_lpstats.emplace(get_self(), [&](auto& row) 
        {
            row.key = "totals"_n;
            
            row.locked = locked;

            row.mining_start_time = now;
            row.halving1_deadline = now+(1*seconds_in_a_month * months_between_halvings);
            row.halving2_deadline = now+(2*seconds_in_a_month * months_between_halvings);
            row.halving3_deadline = now+(3*seconds_in_a_month * months_between_halvings);
            row.halving4_deadline = now+(4*seconds_in_a_month * months_between_halvings);
            row.last_reward_time = now;

            row.hub_issue_frequency = hub_issue_frequency;
            row.dop_issue_frequency = dop_issue_frequency;
            row.dmd_issue_frequency = dmd_issue_frequency;
        });
    }
    else
        total_lpstats.modify(total_it, get_self(),[&]( auto& row) 
        { // We can always modify these rows with set() at a later date:
            row.locked = locked;
            row.hub_issue_frequency = hub_issue_frequency;
            row.dop_issue_frequency = dop_issue_frequency;
            row.dmd_issue_frequency = dmd_issue_frequency;
            row.last_reward_time = now;
        });
}

void efimine::issue()
{
    require_auth( "worker.efi"_n );

    /* Mining rate calculations */
    totaltable total_lpstats(get_self(), "totals"_n.value);
    auto total_it = total_lpstats.find("totals"_n.value);
    check(total_it != total_lpstats.end(), "error: totals table is not initiated");
    bool locked = total_lpstats->locked;
    check(locked == true, "error: LP mining is currently inactive. Please come back later.");

    /* Determine halving handicap */
    uint32_t now = current_time_point().sec_since_epoch();

    if (now < halving1_deadline)
        mining_rate_handicap = 1;

    if (now >= halving1_deadline)
        mining_rate_handicap = 2;

    if (now >= halving2_deadline)
        mining_rate_handicap = 4;

    if (now >= halving3_deadline)
        mining_rate_handicap = 8;

    if (now >= halving4_deadline)
        mining_rate_handicap = 16;

    uint32_t last_reward_time     = total_lpstats->last_reward_time;
    uint16_t issue_precision      = 10000; // This means that if we set ("issue_frequency" == 100): we release 0.01 token per second.
                                           //          and if we set ("issue_frequency" == 1):   we release 0.0001 tokens per second.

    // Coins issued every second. Multiplied by 10000 for tokens with precision 4. Divided by "issue_precision" for extra control:
    uint32_t hub_issue_frequency  = total_lpstats->hub_issue_frequency*10000 / issue_precision / mining_rate_handicap;
    uint32_t dop_issue_frequency  = total_lpstats->dop_issue_frequency*10000 / issue_precision / mining_rate_handicap;
    uint32_t dmd_issue_frequency  = total_lpstats->dmd_issue_frequency*10000 / issue_precision / mining_rate_handicap;

    // How many seconds have passed until now and last_reward_time:
    uint32_t seconds_passed = now - last_reward_time; 
    eosio::print_f("Seconds passed since last issue(): [%] \n",seconds_passed);

    // Determine how much total reward should be issued in this period for each of the coins: hub, dmd, dop.
    uint64_t total_hub_released = seconds_passed * hub_issue_frequency;
    uint64_t total_dop_released = seconds_passed * dop_issue_frequency;
    uint64_t total_dmd_released = seconds_passed * dmd_issue_frequency;

    lptable registered_accounts(get_self(), get_self().value);

    /* Get total registered lptokens for this issue cycle */
    auto current_iteration = registered_accounts.begin();
    auto end_itr = registered_accounts.end();

    uint64_t dop_total_lptokens = 0;
    uint64_t hub_total_lptokens = 0;
    uint64_t dmd_total_lptokens = 0;
    while (current_iteration != end_itr)
    {
        dop_total_lptokens += get_asset_amount(current_iteration->owner_account, dop_box_lp_symbol).amount;
        hub_total_lptokens += get_asset_amount(current_iteration->owner_account, hub_box_lp_symbol).amount;
        dmd_total_lptokens += get_asset_amount(current_iteration->owner_account, dmd_box_lp_symbol).amount;

        ++current_iteration;
    }

    eosio::print_f("Finished counting total lptokens for this issue cycle.\n");
    eosio::print_f("dop_total_lptokens: [%]\n",dop_total_lptokens);
    eosio::print_f("hub_total_lptokens: [%]\n",hub_total_lptokens);
    eosio::print_f("dmd_total_lptokens: [%]\n",dmd_total_lptokens);

    auto current_iteration = registered_accounts.begin();
    auto end_itr = registered_accounts.end();

    /* Loop again and give every user their proper rewards */
    while (current_iteration != end_itr)
    {
        name current_owner = current_iteration->owner_account;
        eosio::print_f("Checking lptoken information for: [%]\n",current_iteration->owner_account);
        /* Check the Defibox LP tables to see how many LPTokens each user has */
        asset hub_lptokens = get_asset_amount(current_iteration->owner_account, hub_box_lp_symbol);
        asset dop_lptokens = get_asset_amount(current_iteration->owner_account, dop_box_lp_symbol);
        asset dmd_lptokens = get_asset_amount(current_iteration->owner_account, dmd_box_lp_symbol);

        uint64_t hub_current_snapshot = hub_lptokens.amount;
        uint64_t dop_current_snapshot = dop_lptokens.amount;
        uint64_t dmd_current_snapshot = dmd_lptokens.amount;

        uint64_t hub_previous_snapshot = current_iteration->hub_snapshot_lp_amount;
        uint64_t dop_previous_snapshot = current_iteration->dop_snapshot_lp_amount;
        uint64_t dmd_previous_snapshot = current_iteration->dmd_snapshot_lp_amount;

        uint64_t hub_before_snapshot = current_iteration->hub_before_lp_amount;
        uint64_t dop_before_snapshot = current_iteration->dop_before_lp_amount;
        uint64_t dmd_before_snapshot = current_iteration->dmd_before_lp_amount;

        /* We'll use this to calculate the user's lp rewards and add to his unclaimed_balance */
        uint64_t hub_lp_calculation_amount;
        uint64_t dop_lp_calculation_amount;
        uint64_t dmd_lp_calculation_amount;

        if (hub_current_snapshot > hub_before_snapshot)
            hub_lp_calculation_amount = hub_before_snapshot;
        else
            hub_lp_calculation_amount = hub_current_snapshot;

        if (dop_current_snapshot > dop_before_snapshot)
            dop_lp_calculation_amount = dop_before_snapshot;
        else
            dop_lp_calculation_amount = dop_current_snapshot;

        if (dmd_current_snapshot > dmd_before_snapshot)
            dmd_lp_calculation_amount = dmd_before_snapshot;
        else
            dmd_lp_calculation_amount = dmd_current_snapshot;

        uint64_t hub_unclaimed_amount = 0;
        uint64_t dop_unclaimed_amount = 0;
        uint64_t dmd_unclaimed_amount = 0;
        /* Calculate the user's unclaimed rewards. Then we'll just += the unclaimed_amount */
        if (row.hub_staked_amount.amount > 0)
        {
            float hub_percentage = float(hub_lp_calculation_amount)/float(hub_total_lptokens) * 100; 
            hub_unclaimed_amount = (hub_percentage*total_hub_released)/0.01/10000;} /* (divided by 10000) for coins with precision 4 */
                
        if (row.dop_staked_amount.amount > 0)
        {
            float dop_percentage = float(dop_lp_calculation_amount)/float(dop_total_lptokens) * 100; 
            dop_unclaimed_amount = (dop_percentage*total_dop_released)/0.01/10000;} /* (divided by 10000) for coins with precision 4 */

        if (row.dmd_staked_amount.amount > 0)
        {
            float dmd_percentage = float(dmd_lp_calculation_amount)/float(dmd_total_lptokens) * 100; 
            dmd_unclaimed_amount = (dmd_percentage*total_dmd_released)/0.01/10000;} /* (divided by 10000) for coins with precision 4 */

        /* Modify the table and have before = snapshot and snapshot = current_snapshot */
        /* Add the user's unclaimed rewards if necessary */
        registered_accounts.modify(current_iteration, get_self(), [&](auto& row)
        {
            row.hub_before_lp_amount = hub_previous_snapshot;
            row.dop_before_lp_amount = dop_previous_snapshot;
            row.dmd_before_lp_amount = dmd_previous_snapshot;

            row.hub_snapshot_lp_amount = hub_current_snapshot;
            row.dop_snapshot_lp_amount = dop_current_snapshot;
            row.dmd_snapshot_lp_amount = dmd_current_snapshot;

            row.hub_unclaimed_amount += hub_unclaimed_amount;
            row.dop_unclaimed_amount += dop_unclaimed_amount;
            row.dmd_unclaimed_amount += dmd_unclaimed_amount;
        });

        ++ current_iteration;
    }
    // Think about how this will work together with the NFT mechanics.
    // For extra points, have the lpmine contract ask for coins from another account.
}

void efimine::registeruser(const name& owner_account)
{   // User pays for their own RAM to be added to the table
    // Function should check if the user has a minimum amount of gluedog LP tokens, and if not, they will not be registered.
    require_auth(owner_account);

    /* Check that the user has not already registered */
    lptable registered_accounts(get_self(), get_self().value);
    auto lprewards_it = registered_accounts.find(owner_account.value);
    check(lprewards_it == registered_accounts.end(), "error: User has already been registered.");

    // The minimum amount for each LPToken that the user needs to have in their account to be registered. //
    // These should be configured from the totals table //
    minimum_dop_amount = 100;
    minimum_hub_amount = 100; 
    minimum_dmd_amount = 100;

    asset hub_lptokens = get_asset_amount(begin_itr->owner_account, hub_box_lp_symbol);
    asset dop_lptokens = get_asset_amount(begin_itr->owner_account, dop_box_lp_symbol);
    asset dmd_lptokens = get_asset_amount(begin_itr->owner_account, dmd_box_lp_symbol);

    bool empty = true;

    if ((dop_lptokens.amount >= minimum_dop_amount) || (hub_lptokens.amount >= minimum_hub_amount) || (dmd_lptokens.amount >= minimum_dmd_amount))
    {
        empty = false;
    }

    check(empty == false, "User does not meet the minumum LP size requirement for HUB, DMD or DOP. Please add more liquidity.");
    /* Add the user in the table at this point */
    registered_accounts.emplace(get_self(), [&](auto& row)
    {
        /* Here we'll have the `before` and the `snapshot` LP amount counter */
        /* This is the first time they are added, so most variables are set to zero */
        row.owner_account = owner_account;

        row.hub_snapshot_lp_amount = hub_lptokens.amount;
        row.dop_snapshot_lp_amount = dop_lptokens.amount;
        row.dmd_snapshot_lp_amount = dmd_lptokens.amount;

        row.hub_before_lp_amount = 0;
        row.dop_before_lp_amount = 0;
        row.dmd_before_lp_amount = 0;

        row.dop_claimed_amount = 0;
        row.hub_claimed_amount = 0;
        row.dmd_claimed_amount = 0;

        row.dop_unclaimed_amount = 0;
        row.hub_unclaimed_amount = 0;
        row.dmd_unclaimed_amount = 0;
    });
}

void efimine::claimrewards(const name& owner_account)
{   // User calls this function to have their unclaimed_rewards sent to them.
    // `claimed_rewards += unclaimed_rewards` and `unclaimed_rewards = 0` after a successful claim()

    require_auth(owner_account);
    // Check if user is registered. If not, throw error.
    lptable registered_accounts(get_self(), get_self().value);
    auto lprewards_it = registered_accounts.find(owner_account.value);
    check(lprewards_it != registered_accounts.end(), "error: User is not registered. Please register your account first.");

    bool empty = true;
    if ((lprewards_it->dop_unclaimed_amount > 0) || (lprewards_it->hub_unclaimed_amount > 0) || (lprewards_it->dmd_unclaimed_amount > 0))
    {
        empty = false;
    }
    check(empty == false,"error: No rewards available to claim.")

    // Check the registered user table, update the claimed_amount += unclaimed_amount and then unclaimed_amount and send the users their rewards.
    if (lprewards_it->dop_unclaimed_amount > 0)
    {
        asset dop_reward_amount;
        dop_reward_amount.symbol = dop_symbol;
        dop_reward_amount.amount = lprewards_it->dop_unclaimed_amount;

        registered_accounts.modify(lprewards_it, get_self(), [&](auto& row)
        {
            row.dop_claimed_amount += dop_reward_amount.amount;
            row.dop_unclaimed_amount = 0;
        });

        efimine::inline_transferdop(get_self(), owner_account, dop_reward_amount, "I'm LP mining DOP!!!");
    }

    if (lprewards_it->hub_unclaimed_amount > 0)
    {
        asset hub_reward_amount;
        hub_reward_amount.symbol = hub_symbol;
        hub_reward_amount.amount = lprewards_it->hub_unclaimed_amount;

        registered_accounts.modify(lprewards_it, get_self(), [&](auto& row)
        {
            row.hub_claimed_amount += hub_reward_amount.amount;
            row.hub_unclaimed_amount = 0;
        });
        
        efimine::inline_transferhub(get_self(), owner_account, hub_reward_amount, "I'm LP mining HUB!!!");
    }

    if (lprewards_it->dmd_unclaimed_amount > 0)
    {
        asset dmd_reward_amount;
        dmd_reward_amount.symbol = dmd_symbol;
        dmd_reward_amount.amount = lprewards_it->dmd_unclaimed_amount;

        registered_accounts.modify(lprewards_it, get_self(), [&](auto& row)
        {
            row.dmd_claimed_amount += dmd_reward_amount.amount;
            row.dmd_unclaimed_amount = 0;
        });
        
        efimine::inline_transferdmd(get_self(), owner_account, dmd_reward_amount, "I'm LP mining DMD!!!");
    }
}
