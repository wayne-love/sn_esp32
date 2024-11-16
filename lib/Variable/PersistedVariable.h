// lib/Variable/PersistentVariable.h
#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "Variable.h"

/**
 * @brief Template class for variables that persist their values in flash memory
 * @tparam T The type of value to store (int, float, double, bool, or String)
 * @extends Variable<T>
 */
template<typename T>
class PersistentVariable : public Variable<T> {
private:
    Preferences prefs;
    String key;             // Storage key name
    String namespace_name;  // Preferences namespace

    /**
     * @brief Saves value to flash memory using type-specific Preferences method
     * @param value The value to persist
     */
    void saveValue(const T& value) {
        prefs.begin(namespace_name.c_str(), false);
        // Choose appropriate storage method based on type
        if constexpr (std::is_same_v<T, int>) {
            prefs.putInt(key.c_str(), value);
        }
        else if constexpr (std::is_same_v<T, float>) {
            prefs.putFloat(key.c_str(), value);
        }
        else if constexpr (std::is_same_v<T, double>) {
            prefs.putDouble(key.c_str(), value);
        }
        else if constexpr (std::is_same_v<T, bool>) {
            prefs.putBool(key.c_str(), value);
        }
        else if constexpr (std::is_same_v<T, String>) {
            prefs.putString(key.c_str(), value);
        }
        prefs.end();
    }

    /**
     * @brief Loads value from flash memory using type-specific Preferences method
     * @return The loaded value, or type-appropriate default if not found
     */
    T loadValue() {
        prefs.begin(namespace_name.c_str(), true);
        T result;
        // Choose appropriate loading method based on type
        if constexpr (std::is_same_v<T, int>) {
            result = prefs.getInt(key.c_str(), 0);
        }
        else if constexpr (std::is_same_v<T, float>) {
            result = prefs.getFloat(key.c_str(), 0.0f);
        }
        else if constexpr (std::is_same_v<T, double>) {
            result = prefs.getDouble(key.c_str(), 0.0);
        }
        else if constexpr (std::is_same_v<T, bool>) {
            result = prefs.getBool(key.c_str(), false);
        }
        else if constexpr (std::is_same_v<T, String>) {
            result = prefs.getString(key.c_str(), "");
        }
        prefs.end();
        return result;
    }

public:
    /**
     * @brief Creates a persistent variable with given namespace and key
     * @param ns Preferences namespace to use for storage
     * @param key_name Key name to use within namespace
     * @param defaultValue Initial value if not found in storage
     */
    PersistentVariable(const String& ns, const String& key_name, T defaultValue = T())
        : Variable<T>(defaultValue), key(key_name), namespace_name(ns) {
        // Load existing value if available
        if (prefs.begin(namespace_name.c_str(), true)) {
            if (prefs.isKey(key.c_str())) {
                Variable<T>::setValue(loadValue());
            } else {
                saveValue(defaultValue);
            }
            prefs.end();
        }
    }

    /**
     * @brief Overridden setValue to persist changes
     * @param newValue Value to set and persist
     * @return true if value was changed and persisted
     */
    bool setValue(T newValue) override {
        if (Variable<T>::setValue(newValue)) {
            saveValue(newValue);
            return true;
        }
        return false;
    }

    /**
     * @brief Overridden string-based setValue to persist changes
     * @param strValue String value to convert, set and persist
     * @return true if value was changed and persisted
     */
    bool setValue(const String& strValue) override {
        if (Variable<T>::setValue(strValue)) {
            saveValue(Variable<T>::getValue());
            return true;
        }
        return false;
    }
};