/*

DMD Yeld farms.

1. DMD Vault where users stake DMD for 3-6-9 months and get DMD rewards.

2. The Farms themselves will require users to register for mining.

3. Not sure if we should have a separate pool for each farm or a single smart contract with multiple tables and multiple settings.
        We will most likely just have admin settings where we'll configure the pools and variables like BOX-LP token symbol.

4. Don't forget about the NFTs.

*/

void dmdfarms::setpool(uint16_t pool_id, uint32_t dmd_issue_frequency, bool is_active, uint64_t min_lp_tokens, const& asset box_asset_symbol, const& string pool_name, uint64_t dmd_mine_qty_remaining)
{
    require_auth(get_self());

    totaltable pool_stats(get_self(), pool_id.value);
    auto total_it = pool_stats.find(pool_id.value);

    uint32_t now = current_time_point().sec_since_epoch();
    uint32_t seconds_in_a_month = 2629743;
    uint32_t months_between_halvings = 3;

    if(total_it == pool_stats.end()) 
    { /* Some of these rows will only get modified the first time setpool() is ran */
        pool_stats.emplace(get_self(), [&](auto& row) 
        {
            row.key = pool_id;
            
            row.is_active = is_active;
            /* We can keep the halvings, and these would be applied separately, to each pool. Not a bad thing to keep. */
            row.mining_start_time = now;
            row.halving1_deadline = now+(1*seconds_in_a_month * months_between_halvings);
            row.halving2_deadline = now+(2*seconds_in_a_month * months_between_halvings);
            row.halving3_deadline = now+(3*seconds_in_a_month * months_between_halvings);
            row.halving4_deadline = now+(4*seconds_in_a_month * months_between_halvings);
            row.last_reward_time = now;

            row.dmd_mine_qty_remaining = dmd_mine_qty_remaining; /* How many DMDs are left to be mined in the pool */
            row.dmd_issue_frequency = dmd_issue_frequency;
            row.minimum_lp_tokens = min_lp_tokens;   /* Minimum LP tokens required to earn yield in the pool. Needs to check for this on issue(). */
                                                     /* Should set the minimum to be around 100-200 EOS liquidity, because we don't want to get spammed */
                                                     /* Should implement automatic user unregistering */
            row.box_asset_symbol = box_asset_symbol; /* The BOX-LP Tokens used to identify the pair for the pool. */
            row.pool_name = pool_name;               /* String identifying the pool name, for display purposes only. */
        });
    }
    else
        pool_stats.modify(total_it, get_self(),[&]( auto& row) 
        { // We can always modify these rows with set() at a later date:
            row.is_active = is_active;

            row.dmd_issue_frequency = dmd_issue_frequency;
            row.dmd_mine_qty_remaining = dmd_mine_qty_remaining; /* How many DMDs are left to be mined in the pool */
            row.minimum_lp_tokens = min_lp_tokens;
            row.box_asset_symbol = box_asset_symbol;
            row.pool_name = pool_name;

            row.last_reward_time = now;
        });
}

void efimine::issue(uint16_t pool_id) /* Should add an offset here to control batching */
{   /* The worker will do an issue for each individual pool. */
    require_auth("worker.efi"_n);

    /* Mining rate calculations */
    totaltable pool_stats(get_self(), pool_id.value);
    auto total_it = pool_stats.find(pool_id.value);
    check(total_it != pool_stats.end(), "error: pool_id not found.");
    check(pool_stats->is_active == true, "error: specified pool is inactive.");

    uint8_t mining_rate_handicap;
    uint32_t now = current_time_point().sec_since_epoch();
    /* Determine halving handicap */
    if (now  < pool_stats->halving1_deadline)  {  mining_rate_handicap = 1;  }
    if (now >= pool_stats->halving1_deadline)  {  mining_rate_handicap = 2;  }
    if (now >= pool_stats->halving2_deadline)  {  mining_rate_handicap = 4;  }
    if (now >= pool_stats->halving3_deadline)  {  mining_rate_handicap = 8;  }
    if (now >= pool_stats->halving4_deadline)  {  mining_rate_handicap = 16; }

    uint16_t issue_precision  = 10000; // With our current algorithm: if we set ("issue_frequency" == 100): we release 0.01 tokens per second.
                                      //    and if we set ("issue_frequency" == 1):   we release 0.0001 tokens per second.

    /* How many coins are issued every second. Multiplied by 10000 for tokens with precision 4. Divided by "issue_precision" for extra control */
    uint32_t augmented_dmd_issue_frequency = pool_stats->dmd_issue_frequency*10000 / issue_precision / mining_rate_handicap;

    // How many seconds have passed until now and last_reward_time:
    uint32_t seconds_passed = now - pool_stats->last_reward_time; 
    eosio::print_f("Seconds passed since last issue(): [%] \n",seconds_passed);

    /* Determine how much total DMD reward should be issued in this transaction/cycle/block */
    uint64_t total_dmd_released = seconds_passed * augmented_dmd_issue_frequency;

    lptable registered_accounts(get_self(), pool_id.value);

    /* Get total registered lptokens for this issue cycle */
    auto current_iteration = registered_accounts.begin();
    auto end_itr = registered_accounts.end();

    uint64_t pool_total_lptokens = 0; /* Counting the total LP Tokens of everyone currently mining in pool ("pool_id") */
    while (current_iteration != end_itr)
    {
        pool_total_lptokens += get_asset_amount(current_iteration->owner_account, dmd_box_lp_symbol).amount;
        /* Need to get SYMBOL from asset. */
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

        uint64_t dmd_current_snapshot  = dmd_lptokens.amount;                       /* Must definitely test this. */
        uint64_t dmd_previous_snapshot = current_iteration->dmd_snapshot_lp_amount; /* Must definitely test this. */
        uint64_t dmd_before_snapshot   = current_iteration->dmd_before_lp_amount;   /* Must definitely test this. */

        /* We'll use this to calculate the user's lp rewards and add to his unclaimed_balance */
        uint64_t dmd_lp_calculation_amount; /* We'll use whatever is lower between current_snapshot and before_snapshot. */

        if (dmd_current_snapshot > dmd_before_snapshot)
            dmd_lp_calculation_amount = dmd_before_snapshot;
        else
            dmd_lp_calculation_amount = dmd_current_snapshot;

        uint64_t dmd_unclaimed_amount = 0;
        /* Calculate the user's unclaimed rewards. Then we'll just += the unclaimed_amount */

        if (dmd_lp_calculation_amount > 0)
        {   float dmd_percentage = float(dmd_lp_calculation_amount)/float(dmd_total_lptokens) * 100; 
            dmd_unclaimed_amount = (dmd_percentage*total_dmd_released)/0.01/10000;  } /* (divided by 10000) for coins with precision 4 */

        /* Modify the table and have before = snapshot and snapshot = current_snapshot */
        /* Add the user's unclaimed rewards if necessary */
        registered_accounts.modify(current_iteration, get_self(), [&](auto& row)
        {
            row.dmd_before_lp_amount = dmd_previous_snapshot;  /* Must definitely test this. */
            row.dmd_snapshot_lp_amount = dmd_current_snapshot; /* Must definitely test this. */
            row.dmd_unclaimed_amount += dmd_unclaimed_amount;  /* Must definitely test this. */
        });

        ++ current_iteration;
    }
    // Think about how this will work together with the NFT mechanics.
    /* The NFTs calculation should be simple. We will check to see if user has or has not got NFT, and we will add a +10% to his farming bonus at the end. */
    // For extra points, have the lpmine contract ask for coins from another account.
}

void efimine::registeruser(const name& owner_account, uint16_t pool_id)
{   /* User pays for their own RAM to be added to the table. Users will have to register to each pool separately. */
    /* Function will check if the user has a minimum amount of LP tokens, and if not, they will not be registered for the pool. */
    require_auth(owner_account);

    /* Check pool integrity */
    totaltable pool_stats(get_self(), pool_id.value);
    auto total_it = pool_stats.find(pool_id.value);
    check(total_it != pool_stats.end(), "error: pool_id not found.");
    check(pool_stats->is_active == true, "error: specified pool is inactive.");

    /* Check that the user has not already registered */
    lptable registered_accounts(get_self(), pool_id.value); /* Need to fine-tune this syntax */
    auto registered_it = registered_accounts.find(pool_id.value); /* Need to fine-tune this syntax */
    check(registered_it == registered_accounts.end(), "error: User has already been registered for the pool.");
    /* Check to see if this is the first user registering for the table with the specific pool ID or not, and either modify or emplace */

    /* Probably need to change this syntax and make a loop for all the rows that fall within pool_id scope and see if we can find owner_account */

    // The minimum amount for each LPToken that the user needs to have in their account to be registered. //
    // These should be configured from the totals table //

    asset pool_lptokens = get_asset_amount(owner_account, pool_stats->asset_symbol);

    bool empty = true;
    if (pool_lptokens.amount >= pool_stats->minimum_lp_tokens;)
        empty = false;

    check(empty == false, "User does not meet the minumum LP size requirement for the specific pool. Please add more liquidity.");
    /* Add the user in the table at this point */
    registered_accounts.emplace(get_self(), [&](auto& row)
    {
        /* Here we'll have the `before` and the `snapshot` LP amount counter. */
        /* This is the first time they are added, so most variables are set to zero. */
        row.owner_account = owner_account;
        row.boxlptoken_snapshot_amount = pool_lptokens.amount;
        row.boxlptoken_before_amount = 0;
        row.dmd_claimed_amount = 0;
        row.dmd_unclaimed_amount = 0;
    });
}

void efimine::claimrewards(const name& owner_account, uint16_t pool_id)
{   
    require_auth(owner_account);

    /* Check pool integrity */
    totaltable pool_stats(get_self(), pool_id.value);
    auto total_it = pool_stats.find(pool_id.value);
    check(total_it != pool_stats.end(), "error: Specified mining pool is not valid.");

    /* Check if user is registered and if he has a claimable balance */
    lptable registered_accounts(get_self(), pool_id.value);
    auto lprewards_it = registered_accounts.find(owner_account.value);
    check(lprewards_it != registered_accounts.end(), "error: User is not registered.");
    check(lprewards_it->dmd_unclaimed_amount > 0, "error: No rewards available to claim.");

    /* Check the registered user table, update the claimed_amount and unclaimed_amount */
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
        /* Send rewards */
        efimine::inline_transferdmd(get_self(), owner_account, dmd_reward_amount, "I'm LP mining DMD in the Yield Farms !");
        /* Update the "dmd_remaining" variable for the pools "totaltable" */
        pool_stats.modify(total_it, get_self(),[&]( auto& row) 
        {  row.dmd_mine_qty_remaining -= dmd_reward_amount.amount;  });
    }
}
