// lib/Variable/Variable.h
#pragma once
#include <Arduino.h>
#include <Preferences.h>

/**
 * @brief A template class for managing a value with update callbacks
 * @tparam T The type of value to store
 */
template<typename T>
class Variable {
private:
    T value;
    bool (*beforeUpdate)(T) = nullptr;    // Called before update with newValue, returns true to allow change
    void (*onUpdate)(T) = nullptr;        // Called after successful update with newValue
    T (*stringConverter)(const String&) = nullptr;  // Allows custom string to T conversion

    // Default converter implementations
    static T defaultStringConverter(const String& str) {
        if constexpr (std::is_same_v<T, int>) {
            return str.toInt();
        }
        else if constexpr (std::is_same_v<T, float>) {
            return str.toFloat();
        }
        else if constexpr (std::is_same_v<T, double>) {
            return str.toDouble();
        }
        else if constexpr (std::is_same_v<T, long>) {
            return str.toInt();
        }
        else if constexpr (std::is_same_v<T, String>) {
            return str;
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return str.equalsIgnoreCase("true") || str == "1";
        }
        else {
            return T(); // Default construct for unsupported types
        }
    }

protected:
    /**
     * @brief Sets the callback function to be called before value updates
     * @param newCallback Function pointer to call before updates, returns true to allow change
     */
    void setBeforeUpdate(bool (*newCallback)(T)) {
        beforeUpdate = newCallback;
    }

    /**
     * @brief Clears the beforeUpdate callback
     */
    void clearBeforeUpdate() {
        beforeUpdate = nullptr;
    }

    /**
     * @brief Sets the string conversion function for this type
     * @param converter Function that converts String to type T
     */
    static void setStringConverter(T (*converter)(const String&)) {
        stringConverter = converter;
    }

public:
    /**
     * @brief Constructs a new Variable with an initial value and optional beforeUpdate callback
     * @param initialValue The initial value to store (default constructed if not provided)
     * @param beforeUpdateCallback Optional callback to validate updates (nullptr if not provided)
     */
    Variable(T initialValue = T(), bool (*beforeUpdateCallback)(T) = nullptr) 
        : value(initialValue), beforeUpdate(beforeUpdateCallback) {}

    /**
     * @brief Gets the current stored value
     * @return The current value
     */
    T getValue() const { return value; }
    
    /**
     * @brief Attempts to set a new value, triggering callbacks if value changes
     * @param newValue The value to set
     * @return true if value was updated, false if unchanged or update prevented
     */
    bool setValue(T newValue) {
        if (value != newValue) {
            bool allowUpdate = true;
            if (beforeUpdate) {
                allowUpdate = beforeUpdate(newValue);
            }
            if (allowUpdate) {
                value = newValue;
                if (onUpdate) {
                    onUpdate(value);
                }
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Attempts to set value from string using configured converter
     * @param strValue String value to convert and set
     * @return true if value was updated, false if unchanged, invalid, or prevented
     */
    bool setValue(const String& strValue) {
        if (stringConverter) {
            return setValue(stringConverter(strValue));
        }
        return setValue(defaultStringConverter(strValue));
    }

    /**
     * @brief Sets the callback function to be called after value updates
     * @param newCallback Function pointer to call after successful updates
     */
    void setOnUpdate(void (*newCallback)(T)) {
        onUpdate = newCallback;
    }

    /**
     * @brief Clears the onUpdate callback
     */
    void clearOnUpdate() {
        onUpdate = nullptr;
    }
};

/**
 * @brief Template class for variables that persist their values in flash memory
 * @tparam T The type of value to store (int, float, double, bool, or String)
 * @extends Variable<T>
 */
template<typename T>
class PersistedVariable : public Variable<T> {
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
    PersistedVariable(const String& ns, const String& key_name, T defaultValue = T())
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


/**
 * @brief A variable that can only be modified by its containing class
 * @tparam T The type of value to store
 */
template<typename T>
class ReadOnlyVariable : public Variable<T> {

protected:
    using Variable<T>::setValue;  // Bring setValue into protected scope
    using Variable<T>::setOnUpdate; // Bring setOnUpdate into protected scope
    using Variable<T>::setBeforeUpdate; // Bring setBeforeUpdate into protected scope
    using Variable<T>::clearOnUpdate; // Bring clearOnUpdate into protected scope
    using Variable<T>::clearBeforeUpdate; // Bring clearBeforeUpdate into protected scope   
    using Variable<T>::stringConverter; // Bring stringConverter into protected scope

public:
    /**
     * @brief Constructs a new ReadOnlyVariable
     * @param initialValue Initial value (default constructed if not provided)
     * @param beforeUpdateCallback Optional validation callback (nullptr if not provided)
     */
    ReadOnlyVariable(T initialValue = T(), bool (*beforeUpdateCallback)(T) = nullptr)
        : Variable<T>(initialValue, beforeUpdateCallback) {}

    // Inherit public getValue
    using Variable<T>::getValue;
};
