class OECasinoCurrency
{
    protected ref map<string, int> m_Values;

    void OECasinoCurrency(map<string, int> values)
    {
        m_Values = values;
    }

    int GetBalance(PlayerBase player)
    {
        if (!player || !m_Values || m_Values.Count() == 0) return 0;

        array<EntityAI> items = new array<EntityAI>;
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

        int total = 0;
        foreach (EntityAI entity : items)
        {
            ItemBase item = ItemBase.Cast(entity);
            if (!item) continue;

            int unitValue = 0;
            if (!m_Values.Find(item.GetType(), unitValue) || unitValue <= 0) continue;

            float qty = item.GetQuantity();
            if (qty < 1) qty = 1;
            total += unitValue * Math.Floor(qty);
        }
        return total;
    }

    bool TryDebit(PlayerBase player, int amount)
    {
        if (amount <= 0) return false;
        int balance = GetBalance(player);
        if (balance < amount) return false;
        return RebuildBalance(player, balance - amount);
    }

    bool Credit(PlayerBase player, int amount)
    {
        if (amount <= 0) return true;
        int balance = GetBalance(player);
        return RebuildBalance(player, balance + amount);
    }

    protected bool RebuildBalance(PlayerBase player, int newBalance)
    {
        if (!player || !m_Values || m_Values.Count() == 0) return false;

        array<EntityAI> items = new array<EntityAI>;
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

        foreach (EntityAI entity : items)
        {
            ItemBase item = ItemBase.Cast(entity);
            if (!item) continue;
            int ignored = 0;
            if (m_Values.Find(item.GetType(), ignored))
                GetGame().ObjectDelete(item);
        }

        if (newBalance <= 0) return true;

        int remaining = newBalance;
        while (remaining > 0)
        {
            string bestType = "";
            int bestValue = 0;

            foreach (string typeName, int value : m_Values)
            {
                if (value > 0 && value <= remaining && value > bestValue)
                {
                    bestType = typeName;
                    bestValue = value;
                }
            }

            if (bestValue <= 0 || bestType == "") return false;

            EntityAI created = player.GetHumanInventory().CreateInInventory(bestType);
            if (!created && !player.GetHumanInventory().GetEntityInHands())
                created = player.GetHumanInventory().CreateInHands(bestType);
            if (!created)
                created = player.SpawnEntityOnGroundPos(bestType, player.GetPosition());
            if (!created) return false;

            ItemBase newItem = ItemBase.Cast(created);
            int stackCount = 1;
            if (newItem)
            {
                int maxQty = Math.Floor(newItem.GetQuantityMax());
                if (maxQty > 1)
                    stackCount = Math.Min(maxQty, Math.Floor(remaining / bestValue));

                if (newItem.GetQuantityMax() > 0)
                    newItem.SetQuantity(stackCount);
            }

            remaining -= (bestValue * stackCount);
        }

        return true;
    }
}
