collarPackage = {
    { "timestamp", format.uint32 },
    { "longitude", format.uint32 },
    { "latitude",  format.uint32 },
    { "event",     format.uint8  },
    { "rssi", format.uint8 },
    { "battery", format.uint8 }
}

uplink.setFormat(collarPackage, "collar")

bolusPackage = {
    { "timestamp", format.uint32 },
    { "condition", format.uint8 },
    { "rssi", format.uint8 },
    { "battery", format.uint8 }
}

uplink.setFormat(bolusPackage, "bolus")

-- Called on uplink message, parsed with specified format upper
function onUplink(package, profile)
    print("onUplink called for profile:", profile)
    print( "timestamp:", tools.ts2string(package.timestamp) )

    if profile == "collar" then
        latf, lonf = tools.geo2float(package.latitude, package.longitude)
        print("latitude:", latf)
        print("longitude:", lonf)
        print("event:", package.event)
    elseif profile == "bolus" then
        print("condition:", package.condition)
    end
end

