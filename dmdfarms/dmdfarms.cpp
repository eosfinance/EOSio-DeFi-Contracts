/*

DMD Yeld farms.

1. DMD Vault where users stake DMD for 3-6-9 months and get DMD rewards.

2. The Farms themselves will require users to register for mining.

3. Not sure if we should have a separate pool for each farm or a single smart contract with multiple tables and multiple settings.
        We will most likely just have admin settings where we'll configure the pools and variables like BOX-LP token symbol.

4. Don't forget about the NFTs.

*/


void dmdfarms::setpool(uint16_t pool_id, uint32_t dmd_issue_frequency, bool locked, uint64_t min_lp_tokens, const& asset asset_symbol, const& string pool_name)
{
    require_auth(get_self());

    totaltable pool_stats(get_self(), pool_id.value);
    auto total_it = pool_stats.find(pool_id.value);

    uint32_t now = current_time_point().sec_since_epoch();
    uint32_t seconds_in_a_month = 2629743;
    uint32_t months_between_halvings = 3;

    if(total_it == pool_stats.end()) 
    { // Some of these rows will only get modified the first time set() is ran:
        pool_stats.emplace(get_self(), [&](auto& row) 
        {
            row.key = pool_id;
            
            row.locked = locked;
            /* We can keep the halvings, and these would be applied separately, to each pool. Not a bad thing to keep. */
            row.mining_start_time = now;
            row.halving1_deadline = now+(1*seconds_in_a_month * months_between_halvings);
            row.halving2_deadline = now+(2*seconds_in_a_month * months_between_halvings);
            row.halving3_deadline = now+(3*seconds_in_a_month * months_between_halvings);
            row.halving4_deadline = now+(4*seconds_in_a_month * months_between_halvings);
            row.last_reward_time = now;

            row.dmd_issue_frequency = dmd_issue_frequency;
            row.minimum_lp_tokens = min_lp_tokens; /* Minimum LP tokens required to earn yield in the pool. */
            row.asset_symbol = asset_symbol; /* The BOX-LP Tokens used to identify the pair for the pool. */
            row.pool_name = pool_name; /* String identifying the pool name, for display purposes only */
        });
    }
    else
        pool_stats.modify(total_it, get_self(),[&]( auto& row) 
        { // We can always modify these rows with set() at a later date:
            row.locked = locked;

            row.dmd_issue_frequency = dmd_issue_frequency;
            row.minimum_lp_tokens = min_lp_tokens;
            row.asset_symbol = asset_symbol;

            row.last_reward_time = now;
        });
}

void efimine::issue(uint16_t pool_id)
{   /* The worker will do an issue for every pool, to help with the batching */
    require_auth( "worker.efi"_n );

    /* Mining rate calculations */
    totaltable pool_stats(get_self(), pool_id.value);
    auto total_it = pool_stats.find(pool_id.value);
    check(total_it != pool_stats.end(), "error: pool_id not found.");
    bool locked = pool_stats->locked;
    check(locked == true, "error: The DMD Yield Farms are inactive. Please come back later.");

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

    uint32_t last_reward_time = pool_stats->last_reward_time;
    uint16_t issue_precision  = 10000; // This means that if we set ("issue_frequency" == 100): we release 0.01 token per second.
                                           //          and if we set ("issue_frequency" == 1):   we release 0.0001 tokens per second.

    /* How many coins are issued every second. Multiplied by 10000 for tokens with precision 4. Divided by "issue_precision" for extra control */
    uint32_t dmd_issue_frequency  = pool_stats->dmd_issue_frequency*10000 / issue_precision / mining_rate_handicap;

    // How many seconds have passed until now and last_reward_time:
    uint32_t seconds_passed = now - last_reward_time; 
    eosio::print_f("Seconds passed since last issue(): [%] \n",seconds_passed);

    // Determine how much total reward should be issued in this period for each of the coins: hub, dmd, dop.
    uint64_t total_dmd_released = seconds_passed * dmd_issue_frequency;

    lptable registered_accounts(get_self(), get_self().value);

    /* Get total registered lptokens for this issue cycle */
    auto current_iteration = registered_accounts.begin();
    auto end_itr = registered_accounts.end();

    uint64_t dmd_total_lptokens = 0;
    while (current_iteration != end_itr)
    {
        dmd_total_lptokens += get_asset_amount(current_iteration->owner_account, dmd_box_lp_symbol).amount;

        ++current_iteration;
    }

    eosio::print_f("Finished counting total lptokens for this issue cycle.\n");
    eosio::print_f("dmd_total_lptokens: [%]\n",dmd_total_lptokens);

    auto current_iteration = registered_accounts.begin();
    auto end_itr = registered_accounts.end();

    /* Loop again and give every user their proper rewards */
    while (current_iteration != end_itr)
    {
        name current_owner = current_iteration->owner_account;
        eosio::print_f("Checking lptoken information for: [%]\n",current_iteration->owner_account);
        /* Check the Defibox LP tables to see how many LPTokens each user has */
        asset dmd_lptokens = get_asset_amount(current_iteration->owner_account, dmd_box_lp_symbol);

        uint64_t dmd_current_snapshot = dmd_lptokens.amount;

        uint64_t dmd_previous_snapshot = current_iteration->dmd_snapshot_lp_amount;

        uint64_t dmd_before_snapshot = current_iteration->dmd_before_lp_amount;

        /* We'll use this to calculate the user's lp rewards and add to his unclaimed_balance */
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

void efimine::registeruser(const name& owner_account, uint64_t pool_id)
{   // User pays for their own RAM to be added to the table
    // Function should check if the user has a minimum amount of gluedog LP tokens, and if not, they will not be registered.
    require_auth(owner_account);

    /* Check that the user has not already registered */
    lptable registered_accounts(get_self(), get_self().value);
    auto lprewards_it = registered_accounts.find(owner_account.value);
    check(lprewards_it == registered_accounts.end(), "error: User has already been registered.");

    // The minimum amount for each LPToken that the user needs to have in their account to be registered. //
    // These should be configured from the totals table //
    minimum_dmd_amount = 100;

    asset dmd_lptokens = get_asset_amount(begin_itr->owner_account, dmd_box_lp_symbol);

    bool empty = true;

    if (dmd_lptokens.amount >= minimum_dmd_amount)
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
